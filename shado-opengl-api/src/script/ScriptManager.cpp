#include "ScriptManager.h"
#include "debug/Debug.h"
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/exception.h>
#include <mono/metadata/attrdefs.h>
#include <filesystem>
#include <iostream>

#ifdef SHADO_PLATFORM_WINDOWS
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif

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

	void ScriptManager::init(const std::string& path) {
        assemblyPathFallback = path;

		mono_set_dirs(getRootDir("mono/lib").c_str(), getRootDir("mono/etc").c_str());
        domain = mono_jit_init("shado_engine_script");        

        assembly = mono_domain_assembly_open(domain, path.c_str());
        if (!assembly)
            SHADO_CORE_ERROR("Error loading assembly");
        image = mono_assembly_get_image(assembly);

        // Add costum functions


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
    }

	void ScriptManager::reload(const std::string& path) {
        SHADO_CORE_ASSERT(!path.empty(), "Script DLL path cannot be empty");

        shutdown();
        init(path);

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

        const MonoTableInfo* table_info = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);

        int rows = mono_table_info_get_rows(table_info);

        /* For each row, get some of its values */
        for (int i = 0; i < rows; i++)
        {
            ScriptClassDesc desc;

            MonoClass* _class = nullptr;
            uint32_t cols[MONO_TYPEDEF_SIZE];
            mono_metadata_decode_row(table_info, i, cols, MONO_TYPEDEF_SIZE);
            const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);
            const char* name_space = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
            _class = mono_class_from_name(image, name_space, name);
            
            desc.name = name;
            desc.name_space = name_space;
            desc.klass = _class;

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

            class_list.push_back(desc);
        }
        return class_list;
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
        auto fullSignature = getFullSignature(nameOrSignature);
    	MonoMethodDesc* desc = mono_method_desc_new(fullSignature.c_str(), true);
    	MonoMethod* method = mono_method_desc_search_in_image(desc, image);
    	mono_method_desc_free(desc);
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

	std::string ScriptClassInstance::getFullSignature(const std::string& methodNameAndArgs) {
       return description.name_space + "." + description.name + "::" + methodNameAndArgs;
	}

}
