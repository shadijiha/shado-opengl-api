#include "ScriptManager.h"
#include "debug/Debug.h"
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/exception.h>
#include <mono/metadata/attrdefs.h>
#include <filesystem>
#include <iostream>

#include "Events/input.h"
#include "scene/Components.h"
#include "scene/Entity.h"
#include "scene/Scene.h"

#ifdef SHADO_PLATFORM_WINDOWS
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif

#define REGISTER_COMPONENT(X)   registedComponents[#X] = ComponentsCallbacks( \
	[](Entity e) { e.addComponent<X>(); },\
	[](Entity e) { return e.hasComponent<X>(); },\
	[](Entity e) { e.removeComponent<X>(); }\
);

namespace Shado {

    // From https://stackoverflow.com/questions/67971007/how-do-i-make-this-a-relative-path
    static std::string get_current_dir() {
        char buff[FILENAME_MAX]; //create string buffer to hold path
        GetCurrentDir(buff, FILENAME_MAX);
        std::string current_working_dir(buff);
        return current_working_dir;
    }

    static std::string getRootDir(const std::string& relative) {
        std::filesystem::path path = get_current_dir();
        auto parent = path.parent_path();
        return (parent / relative).string();
    }

    static ScriptFieldDesc::Visibility visibilityToEnum(uint32_t flags);

	ScriptClassDesc ScriptClassDesc::fromMonoClass(MonoClass* klass) {
        if (klass == nullptr)
            return {};

        ScriptClassDesc desc;

        MonoClass* _class = klass;
        const char* name = mono_class_get_name(_class);
        const char* name_space = mono_class_get_namespace(_class);

        MonoClass* parent = mono_class_get_parent(_class);

        desc.name = name;
        desc.name_space = name_space;
        desc.klass = _class;
        desc.parent = parent;

        // Get all methodes
        void* iter = NULL;
        MonoMethod* method;
        while (method = mono_class_get_methods(_class, &iter))
        {
            const char* methodName = mono_method_get_name(method);
            auto* methodSignature = mono_method_signature(method);
            char* methodSignDesc = mono_signature_get_desc(methodSignature, 0);

            desc.methodes[std::string(methodName) + "(" + methodSignDesc + ")"] = method;
        }

        void* fieldIt = nullptr;
        MonoClassField* field;
        while (field = mono_class_get_fields(_class, &fieldIt)) {
            const char* fieldName = mono_field_get_name(field);
            MonoType* type = mono_field_get_type(field);
            uint32_t flags = mono_field_get_flags(field) & MONO_FIELD_ATTR_FIELD_ACCESS_MASK;

            ScriptFieldDesc fieldDesc;
            fieldDesc.name = fieldName;
            fieldDesc.type = type;
            fieldDesc.field = field;
            fieldDesc.visibility = visibilityToEnum(flags);

            desc.fields[fieldName] = fieldDesc;
        }

        return desc;
	}

	void ScriptManager::init(const std::string& path) {
        assemblyPathFallback = path;

		mono_set_dirs(getRootDir("mono/lib").c_str(), getRootDir("mono/etc").c_str());
        domain = mono_jit_init("shado_engine_script");        

        assembly = mono_domain_assembly_open(domain, path.c_str());
        if (!assembly)
            SHADO_CORE_ERROR("Error loading assembly");
        image = mono_assembly_get_image(assembly);

        // Add costum functions
        setUpAllInternalCalls();

        // In case of exception
		mono_install_unhandled_exception_hook([](MonoObject* exc, void* user_data) {
			MonoObject* other = NULL;
			MonoString* str = mono_object_to_string(exc, &other);
			SHADO_CORE_WARN("[C++] exception caught: {0}", mono_string_to_utf8(str));
			throw std::exception("hehexd");
		}, nullptr);
    }

    void ScriptManager::shutdown()
    {
        if (domain)
			mono_jit_cleanup(domain);

		MonoDomain* domainToUnload = mono_domain_get();
		if (domainToUnload && domainToUnload != mono_get_root_domain())
		{
		    mono_domain_set(mono_get_root_domain(), false);
		    //mono_thread_pop_appdomain_ref();
		    mono_domain_unload(domainToUnload);
		}

        ScriptManager::registedComponents.clear();
    }

	void ScriptManager::reload(const std::string& path) {
        SHADO_CORE_ASSERT(!path.empty(), "Script DLL path cannot be empty");

        shutdown();
        init(path);

        // Register components
        REGISTER_COMPONENT(SpriteRendererComponent);
        REGISTER_COMPONENT(CircleRendererComponent);
        REGISTER_COMPONENT(CameraComponent);
        REGISTER_COMPONENT(ScriptComponent);

        REGISTER_COMPONENT(RigidBody2DComponent);
        REGISTER_COMPONENT(BoxCollider2DComponent);
        REGISTER_COMPONENT(CircleCollider2DComponent);

        // unload 
        /*MonoDomain* domainToUnload = mono_domain_get();
        if (domainToUnload && domainToUnload != mono_get_root_domain())
        {
            mono_domain_set(mono_get_root_domain(), false);
            //mono_thread_pop_appdomain_ref();
            mono_domain_unload(domainToUnload);
        }*/
	}

	void ScriptManager::addInternalCall(const std::string& methodSignature, const void* func) {
        mono_add_internal_call(methodSignature.c_str(), func);
    }

    void ScriptManager::invokeStaticMethod(const std::string& methodSignature, void** args) {
        MonoMethodDesc* desc = mono_method_desc_new(methodSignature.c_str(), false);
        MonoMethod* method = mono_method_desc_search_in_image(desc, image);

        mono_runtime_invoke(method, nullptr, args, nullptr);

        mono_method_desc_free(desc);
    }

    void ScriptManager::invokeInstanceMethod(const std::string& methodSignature, MonoObject* obj, void** args) {
        MonoMethodDesc* desc = mono_method_desc_new(methodSignature.c_str(), false);
        MonoMethod* method = mono_method_desc_search_in_image(desc, image);

        mono_runtime_invoke(method, obj, args, nullptr);
        mono_method_desc_free(desc);
    }

    ScriptClassInstance ScriptManager::createObject(const ScriptClassDesc& desc, const std::string& constructorSignature, void** args) {
        return {domain, image, desc, constructorSignature, args};
    }

    MonoClass* ScriptManager::getClass(const std::string& namesace, const std::string& klass) {
        return mono_class_from_name(image, namesace.c_str(), klass.c_str());
    }

    MonoMethod* ScriptManager::getMethod(const std::string& methodSignature) {
        MonoMethodDesc* desc = mono_method_desc_new(methodSignature.c_str(), false);
        MonoMethod* method = mono_method_desc_search_in_image(desc, image);
        mono_method_desc_free(desc);
        return method;
    }

	std::list<ScriptClassDesc> ScriptManager::getAssemblyClassList()
    {
        std::list<ScriptClassDesc> class_list;
        if (!image || !assembly) {
            SHADO_CORE_WARN("mono image or assembly are null. Maybe script DLL not set?");
            return class_list;
        }

        const MonoTableInfo* table_info = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
        
        int rows = mono_table_info_get_rows(table_info);

        /* For each row, get some of its values */
        for (int i = 0; i < rows; i++)
        {
            MonoClass* _class = nullptr;
            uint32_t cols[MONO_TYPEDEF_SIZE];
            mono_metadata_decode_row(table_info, i, cols, MONO_TYPEDEF_SIZE);
            const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);
            const char* name_space = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
            _class = mono_class_from_name(image, name_space, name);

            ScriptClassDesc desc = ScriptClassDesc::fromMonoClass(_class);

            /* MonoClass* parent = mono_class_get_parent(_class);

            desc.name = name;
            desc.name_space = name_space;
            desc.klass = _class;
            desc.parent = parent;

            // Get all methodes
            void* iter = NULL;
            MonoMethod* method;
            while (method = mono_class_get_methods(_class, &iter))
            {
                const char* methodName =  mono_method_get_name(method);
                auto* methodSignature = mono_method_signature(method);
                char* methodSignDesc = mono_signature_get_desc(methodSignature, 0);

                desc.methodes[std::string(methodName) + "(" + methodSignDesc + ")"] = method;
            }

            void* fieldIt = nullptr;
            MonoClassField* field;
            while (field = mono_class_get_fields(_class, &fieldIt)) {
                const char* fieldName = mono_field_get_name(field);
                MonoType* type = mono_field_get_type(field);
                uint32_t flags = mono_field_get_flags(field) & MONO_FIELD_ATTR_FIELD_ACCESS_MASK;

                ScriptFieldDesc fieldDesc;
                fieldDesc.name = fieldName;
                fieldDesc.type = type;
                fieldDesc.field = field;
                fieldDesc.visibility = visibilityToEnum(flags);

            	desc.fields[fieldName] = fieldDesc;
            }
            */
            class_list.push_back(desc);
        }
        return class_list;
    }

	std::list<ScriptClassDesc> ScriptManager::getChildrenOf(const std::string& parentName) {
        std::list<ScriptClassDesc> descs;
        for (auto& klazz : getAssemblyClassList())
            if (klazz.parent && mono_class_get_name(klazz.parent) == parentName)
                descs.push_back(klazz);
        return descs;
	}

	ScriptClassDesc ScriptManager::getClassByName(const std::string& name) {
		for (auto& klass : getAssemblyClassList()) {
            if (klass.name == name)
                return klass;
		}

        SHADO_CORE_ASSERT(false, "Invalid class name");
        return ScriptClassDesc();
	}

	bool ScriptManager::hasClass(const std::string& name, const std::string& name_space) {

        for (const auto& desc : getAssemblyClassList()) {
            bool flag = false;
            if (desc.name == name)
                flag = true;

            if (!name_space.empty() && desc.name_space == name_space) {
                return flag && true;
            } else if (flag)
                return flag;                
        }

        return false;
	}

	static ScriptFieldDesc::Visibility visibilityToEnum(uint32_t flags) {
        if (flags == MONO_FIELD_ATTR_PRIVATE)
            return ScriptFieldDesc::Visibility::PRIVATE;
        if (flags == MONO_FIELD_ATTR_FAM_AND_ASSEM)
            return ScriptFieldDesc::Visibility::PROTECTED_AND_INTERNAL;
        if (flags == MONO_FIELD_ATTR_ASSEMBLY)
            return ScriptFieldDesc::Visibility::INTERNAL;
        if (flags == MONO_FIELD_ATTR_FAMILY)
            return ScriptFieldDesc::Visibility::PROTECTED;
        if (flags == MONO_FIELD_ATTR_PUBLIC)
            return ScriptFieldDesc::Visibility::PUBLIC;
        return ScriptFieldDesc::Visibility::PRIVATE;
    }

    /**
     *************************************************
     ************** ScriptClassInstance **************
     *************************************************
     */
    ScriptClassInstance::ScriptClassInstance(MonoDomain* domain, MonoImage* image, ScriptClassDesc desc, const std::string& ctorSignature,
        void** args)
	    : description(desc), image(image), domain(domain)
	{
        MonoObject* my_class_instance = mono_object_new(domain, desc.klass);

        // Execute the costructor
        if (args == nullptr && ctorSignature.empty()) {
            mono_runtime_object_init(my_class_instance);
        } else
        {
            auto* method = getMethod(ctorSignature);
            mono_runtime_invoke(method, my_class_instance, args, nullptr);
        }

        instance = my_class_instance;
    }

	ScriptClassInstance::~ScriptClassInstance() {
	}

	MonoMethod* ScriptClassInstance::getMethod(const std::string& nameOrSignature, bool includesNamespace) {
    	// See if method exists in description
    	if (description.methodes.find(nameOrSignature) != description.methodes.end()) {
    		return description.methodes[nameOrSignature];
    	}

    	// Otherwise get it from assembly
        MonoMethod* method = nullptr;
        {
            auto fullSignature = getFullSignature(nameOrSignature);
            MonoMethodDesc* desc = mono_method_desc_new(fullSignature.c_str(), true);
            method = mono_method_desc_search_in_image(desc, image);
            mono_method_desc_free(desc);
        }

        // Check if parent has it
        if (method == nullptr) {
            auto fullSignature = description.name_space + "." + mono_class_get_name(description.parent) + "::" + nameOrSignature;
            MonoMethodDesc* desc = mono_method_desc_new(fullSignature.c_str(), true);
            method = mono_method_desc_search_in_image(desc, image);
            mono_method_desc_free(desc);            
        }
    	return method;
    }

    ScriptFieldDesc ScriptClassInstance::getField(const std::string& name) {
        if (description.fields.find(name) == description.fields.end())
            return {};
        return description.fields[name];
	}

	MonoObject* ScriptClassInstance::invokeMethod(MonoMethod* method, void** args) {
        return mono_runtime_invoke(method, instance, args, nullptr);
	}

	MonoObject* ScriptClassInstance::getFieldValue(const ScriptFieldDesc& field) {
        return mono_field_get_value_object(domain, field.field, instance);
	}

	MonoObject* ScriptClassInstance::getFieldValue(const std::string& name) {
        return getFieldValue(getField(name));
	}

	void ScriptClassInstance::setFieldValue(const ScriptFieldDesc& field, void* value) {
        mono_field_set_value(instance, field.field, value);
	}

	void ScriptClassInstance::setFieldValue(const std::string& name, void* value) {
        auto field = getField(name);
        setFieldValue(field, value);
	}

	void* ScriptClassInstance::unbox(MonoObject* obj) {
        return mono_object_unbox(obj);
	}

	std::string ScriptClassInstance::getFullSignature(const std::string& methodNameAndArgs) {
       return description.name_space + "." + description.name + "::" + methodNameAndArgs;
	}


    /**
	 *************************************************
	 ************** ScriptManager internal calls **************
	 *************************************************
	 */
    static std::string getTypeName(MonoObject* type);

	void ScriptManager::setUpAllInternalCalls() {
        struct EntityDesc {
            uint64_t id;
            Scene* scene;
        };

#pragma region Entity
        {
            uint64_t (*CreateEntity)(MonoObject*, MonoObject*) = [](MonoObject* typeObj, MonoObject* instance) {
                Entity e = Scene::ActiveScene->createEntity("Empty C# Entity");

                MonoMethod* method = mono_class_get_method_from_name(mono_object_get_class(typeObj), "get_Name", 0);
                MonoString* value = (MonoString*)mono_runtime_invoke(method, typeObj, nullptr, nullptr);
                std::string name = mono_string_to_utf8(value);
                SHADO_CORE_INFO("{0}", name);

                ScriptComponent& component = e.addComponent<ScriptComponent>();
                component.className = name;
                component.object = ScriptClassInstance();
                component.object.domain = domain;
                component.object.image = image;
                component.object.description = getClassByName(name);
                component.object.instance = instance;
                return (uint64_t)e.getComponent<IDComponent>().id;
            };
            addInternalCall("Shado.Entity::CreateEntity", CreateEntity);

            Scene* (*GetActiveScene)() = []() {
                return Scene::ActiveScene.get();
            };
            addInternalCall("Shado.Entity::GetActiveScene()", GetActiveScene);

            void (*AddComponent_Native)(uint64_t,Scene*, MonoObject*) = [](uint64_t id, Scene* scene, MonoObject* type) {
                std::string name = getTypeName(type);
                registedComponents[name].addComponent(scene->getEntityById(id));
            };
            addInternalCall("Shado.Entity::AddComponent_Native", AddComponent_Native);

            bool (*HasComponent_Native)(uint64_t, Scene*, MonoObject*) = [](uint64_t id, Scene* scene, MonoObject* type) {
                std::string name = getTypeName(type);
                return registedComponents[name].hasComponent(scene->getEntityById(id));
            };
            addInternalCall("Shado.Entity::HasComponent_Native", HasComponent_Native);

            void (*Destroy_Native)(uint64_t, Scene*) = [](uint64_t id, Scene* scene) {
                Entity e = scene->getEntityById(id);
                scene->destroyEntity(e);
            };
            addInternalCall("Shado.Entity::Destroy_Native", Destroy_Native);
        }

#pragma endregion

#pragma region Scene
        {
            MonoObject* (*GetPrimaryCameraEntity)(Scene*) = [](Scene* scene) {
                ScriptClassDesc klass = getClassByName("Entity");
                Entity e = scene->getPrimaryCameraEntity();

                if (!e.isValid())
                    return (MonoObject*)nullptr;

                // Create C# object
                uint64_t id = e.getComponent<IDComponent>().id;
                void* args[2] = {
                    &id,
                    Scene::ActiveScene.get()
                };
               
                auto instance = createObject(klass, ".ctor(ulong,intptr)", args);

                return instance.instance;
            };
            addInternalCall("Shado.Scene::GetPrimaryCameraEntity_Native", GetPrimaryCameraEntity);
        }
#pragma endregion

#pragma region TransformComponent
        {
            void (*GetPosition_Native)(uint64_t, glm::vec3&) = [](uint64_t id, glm::vec3& pos) {
                Entity entity = Scene::ActiveScene->getEntityById(id);
                pos = entity.getComponent<TransformComponent>().position;
            };
            addInternalCall("Shado.TransformComponent::GetPosition_Native", GetPosition_Native);

            void (*SetPosition_Native)(uint64_t, glm::vec3&) = [](uint64_t id, glm::vec3& pos) {
                Entity entity = Scene::ActiveScene->getEntityById(id);
                entity.getComponent<TransformComponent>().position = pos;
            };
            addInternalCall("Shado.TransformComponent::SetPosition_Native", SetPosition_Native);

            void (*GetRotation_Native)(uint64_t, glm::vec3&) = [](uint64_t id, glm::vec3& pos) {
                Entity entity = Scene::ActiveScene->getEntityById(id);
                pos = entity.getComponent<TransformComponent>().rotation;
            };
            addInternalCall("Shado.TransformComponent::GetRotation_Native", GetRotation_Native);

            void (*SetRotation_Native)(uint64_t, glm::vec3&) = [](uint64_t id, glm::vec3& pos) {
                Entity entity = Scene::ActiveScene->getEntityById(id);
                entity.getComponent<TransformComponent>().rotation = pos;
            };
            addInternalCall("Shado.TransformComponent::SetRotation_Native", SetRotation_Native);

            void (*GetScale_Native)(uint64_t, glm::vec3&) = [](uint64_t id, glm::vec3& pos) {
                Entity entity = Scene::ActiveScene->getEntityById(id);
                pos = entity.getComponent<TransformComponent>().scale;
            };
            addInternalCall("Shado.TransformComponent::GetScale_Native", GetScale_Native);

            void (*SetScale_Native)(uint64_t, glm::vec3&) = [](uint64_t id, glm::vec3& pos) {
                Entity entity = Scene::ActiveScene->getEntityById(id);
                entity.getComponent<TransformComponent>().scale = pos;
            };
            addInternalCall("Shado.TransformComponent::SetScale_Native", SetScale_Native);
        }

#pragma endregion

#pragma region SpriteRendererComponent
        {
            void (*GetColor_Native)(uint64_t, glm::vec4&) = [](uint64_t id, glm::vec4& pos) {
                Entity entity = Scene::ActiveScene->getEntityById(id);
                pos = entity.getComponent<SpriteRendererComponent>().color;
            };
            addInternalCall("Shado.SpriteRendererComponent::GetColor_Native", GetColor_Native);

            void (*SetColor_Native)(uint64_t, glm::vec4&) = [](uint64_t id, glm::vec4& pos) {
                Entity entity = Scene::ActiveScene->getEntityById(id);
                entity.getComponent<SpriteRendererComponent>().color = pos;
            };
            addInternalCall("Shado.SpriteRendererComponent::SetColor_Native", SetColor_Native);

            void (*SetTexture_Native)(uint64_t, MonoString*) = [](uint64_t id, MonoString* texture) {
                Entity entity = Scene::ActiveScene->getEntityById(id);
                entity.getComponent<SpriteRendererComponent>().texture = CreateRef<Texture2D>(mono_string_to_utf8(texture));
            };
            addInternalCall("Shado.SpriteRendererComponent::SetTexture_Native", SetTexture_Native);

            MonoObject* (*GetTexture_Native)(EntityDesc) = [](EntityDesc entityId) {
                Entity entity = entityId.scene->getEntityById(entityId.id);
                auto& sprite = entity.getComponent<SpriteRendererComponent>();

                if (sprite.texture == nullptr)
                    return (MonoObject*)nullptr;

                auto klass = ScriptManager::getClassByName("Texture2D");
                auto instance = ScriptManager::createObject(klass);

                auto path = mono_string_new(domain, sprite.texture->getFilePath().c_str());
                auto width = sprite.texture->getWidth();
                auto height = sprite.texture->getHeight();
                auto dataFormat = sprite.texture->getDataFormat();
                auto internalFormat = sprite.texture->getInternalFormat();
                auto loaded = sprite.texture->isLoaded();

                instance.invokeMethod(instance.getMethod("set_Path"), path);               
                instance.setFieldValue("Width", &width);
                instance.setFieldValue("Height", &height);
                instance.setFieldValue("DataFormat", &dataFormat);
                instance.setFieldValue("InternalFormat", &internalFormat);
                instance.setFieldValue("IsLoaded", &loaded);

                return instance.instance;
            };
            addInternalCall("Shado.SpriteRendererComponent::GetTexture_Native", GetTexture_Native);
        }
#pragma endregion

#pragma region RigidBody2DComponent
        {
            void (*GetBodyType_Native)(uint64_t, int&) = [](uint64_t id, int& pos) {
                Entity entity = Scene::ActiveScene->getEntityById(id);
                pos = (int)entity.getComponent<RigidBody2DComponent>().type;
            };
            addInternalCall("Shado.RigidBody2DComponent::GetBodyType_Native", GetBodyType_Native);

            void (*SetBodyType_Native)(uint64_t, int&) = [](uint64_t id, int& pos) {
                Entity entity = Scene::ActiveScene->getEntityById(id);
                entity.getComponent<RigidBody2DComponent>().type = (RigidBody2DComponent::BodyType)pos;
            };
            addInternalCall("Shado.RigidBody2DComponent::SetBodyType_Native", SetBodyType_Native);

            void (*GetFixedRotation_Native)(uint64_t, bool&) = [](uint64_t id, bool& pos) {
                Entity entity = Scene::ActiveScene->getEntityById(id);
                pos = entity.getComponent<RigidBody2DComponent>().fixedRotation;
            };
            addInternalCall("Shado.RigidBody2DComponent::GetFixedRotation_Native", GetFixedRotation_Native);

            void (*SetFixedRotation_Native)(uint64_t, bool&) = [](uint64_t id, bool& pos) {
                Entity entity = Scene::ActiveScene->getEntityById(id);
                entity.getComponent<RigidBody2DComponent>().fixedRotation = pos;
            };
            addInternalCall("Shado.RigidBody2DComponent::SetFixedRotation_Native", SetFixedRotation_Native);
        }
#pragma endregion

#pragma region BoxCollider2DComponent
        {
            void (*GetOffset_Native)(uint64_t, glm::vec2&) = [](uint64_t id, glm::vec2& result) {
                Entity entity = Scene::ActiveScene->getEntityById(id);
                result = entity.getComponent<BoxCollider2DComponent>().offset;
            };
            addInternalCall("Shado.BoxCollider2DComponent::GetOffset_Native", GetOffset_Native);

            void (*SetOffset_Native)(uint64_t, glm::vec2&) = [](uint64_t id, glm::vec2& pos) {
                Entity entity = Scene::ActiveScene->getEntityById(id);
                entity.getComponent<BoxCollider2DComponent>().offset = pos;
            };
            addInternalCall("Shado.BoxCollider2DComponent::SetOffset_Native", SetOffset_Native);

            void (*GetSize_Native)(uint64_t, glm::vec2&) = [](uint64_t id, glm::vec2& result) {
                Entity entity = Scene::ActiveScene->getEntityById(id);
                result = entity.getComponent<BoxCollider2DComponent>().size;
            };
            addInternalCall("Shado.BoxCollider2DComponent::GetSize_Native", GetSize_Native);

            void (*SetSize_Native)(uint64_t, glm::vec2&) = [](uint64_t id, glm::vec2& pos) {
                Entity entity = Scene::ActiveScene->getEntityById(id);
                entity.getComponent<BoxCollider2DComponent>().size = pos;
            };
            addInternalCall("Shado.BoxCollider2DComponent::SetSize_Native", SetSize_Native);

            void (*GetFloatVal_Native)(uint64_t, MonoString*, float&) = [](uint64_t id, MonoString* varName, float& result) {
                Entity entity = Scene::ActiveScene->getEntityById(id);
                std::string prop = mono_string_to_utf8(varName);

                if (prop == "density") {
                    result = entity.getComponent<BoxCollider2DComponent>().density;
                } else if (prop == "friction") {
                    result = entity.getComponent<BoxCollider2DComponent>().friction;
                } else if (prop == "restitution") {
                    result = entity.getComponent<BoxCollider2DComponent>().restitution;
                } else if (prop == "restitutionThreshold") {
                    result = entity.getComponent<BoxCollider2DComponent>().restitutionThreshold;
                }

                
            };
            addInternalCall("Shado.BoxCollider2DComponent::GetFloatVal_Native", GetFloatVal_Native);

            void (*SetFloatVal_Native)(uint64_t, MonoString*, float&) = [](uint64_t id, MonoString* varName, float& input) {
                Entity entity = Scene::ActiveScene->getEntityById(id);
                std::string prop = mono_string_to_utf8(varName);

                if (prop == "density") {
                    entity.getComponent<BoxCollider2DComponent>().density = input;
                } else if (prop == "friction") {
                    entity.getComponent<BoxCollider2DComponent>().friction = input;
                } else if (prop == "restitution") {
                    entity.getComponent<BoxCollider2DComponent>().restitution = input;
                } else if (prop == "restitutionThreshold") {
                    entity.getComponent<BoxCollider2DComponent>().restitutionThreshold = input;
                }
            };
            addInternalCall("Shado.BoxCollider2DComponent::SetFloatVal_Native", SetFloatVal_Native);
        }
#pragma endregion 

#pragma region Debug
        {
            void (*Log)(MonoString*, int) = [](MonoString* obj, int type) {
                std::string message = mono_string_to_utf8(obj);
				switch (type) {
					case 0:
                        SHADO_ERROR(message);
	                    break;
					case 1:
                        SHADO_WARN(message);
	                    break;
					case 2:
                        SHADO_INFO(message);
	                    break;
				}
            };
            addInternalCall("Shado.Debug::Log", Log);
        }
#pragma endregion

#pragma region Input
        {
            bool (*IsKeyPressed_Native)(KeyCode) = [](KeyCode keycode) {
                return Input::isKeyPressed(keycode);
            };
            addInternalCall("Shado.Input::IsKeyPressed_Native", IsKeyPressed_Native);

            bool (*IsMouseButtonPressed_Native)(int) = [](int keycode) {
                return Input::isMouseButtonPressed(keycode);
            };
            addInternalCall("Shado.Input::IsMouseButtonPressed_Native", IsMouseButtonPressed_Native);


            void (*GetMousePos_Native)(glm::vec2&) = [](glm::vec2& pos) {
                pos = {Input::getMouseX(), Input::getMouseY()};
            };
            addInternalCall("Shado.Input::GetMousePos_Native", GetMousePos_Native);
        }
#pragma endregion

#pragma region Texture2D
        {
            void (*CreateTexture2D_Native)(MonoString*, uint32_t&, uint32_t&, uint32_t&, uint32_t&, bool&) =
                [](MonoString* path, uint32_t& width, uint32_t& height, uint32_t& dataFormat, uint32_t& internalFormat, bool& loaded) {

                Texture2D* texture = new Texture2D(mono_string_to_utf8(path));
                //native = texture;
                width = texture->getWidth();
                height = texture->getHeight();
                loaded = texture->isLoaded();
                dataFormat = texture->getDataFormat();
                internalFormat = texture->getInternalFormat();

                delete texture;
            };
            addInternalCall("Shado.Texture2D::CreateTexture2D_Native", CreateTexture2D_Native);

            void (*DestroyTexture2D_Native)(Texture2D*) = [](Texture2D* native) {
                delete native;
            };
            addInternalCall("Shado.Texture2D::DestroyTexture2D_Native", DestroyTexture2D_Native);

        }
#pragma endregion

#pragma region Renderer
        {
            void (*DrawQuad_Native)(glm::vec3, glm::vec3, glm::vec3, glm::vec4) =
                [](glm::vec3 pos, glm::vec3 size, glm::vec3 rot, glm::vec4 color) {

                Renderer2D::DrawRotatedQuad(pos, size, rot, color);
            };
            addInternalCall("Shado.Renderer::DrawQuad_Native", DrawQuad_Native);

        }
#pragma endregion
	}
    
    static std::string getTypeName(MonoObject* type) {

        // Get the type class Name
        MonoMethod* method = mono_class_get_method_from_name(mono_object_get_class(type), "get_Name", 0);
        MonoString* value = (MonoString*)mono_runtime_invoke(method, type, nullptr, nullptr);
        std::string name = mono_string_to_utf8(value);
        return name;
    }
}
