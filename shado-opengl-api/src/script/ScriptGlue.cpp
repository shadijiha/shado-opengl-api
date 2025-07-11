#include "ScriptGlue.h"

#include <box2d/box2d.h>
#include <glm/gtc/quaternion.hpp>
#include <imgui.h>
#include <imgui_internal.h>
#include <glm/glm.hpp>

#include <functional>

#include <Coral/HostInstance.hpp>

#include "ScriptEngine.h"
#include "asset/AssetManager.h"
#include "Events/input.h"
#include "project/Project.h"
#include "scene/Prefab.h"
#include "util/TypeInfo.h"

namespace Shado {
#ifdef SHADO_PLATFORM_WINDOWS
#define HZ_FUNCTION_NAME __func__
#else
#define HZ_FUNCTION_NAME __FUNCTION__
#endif

#define SHADO_ADD_INTERNAL_CALL(icall) coreAssembly.AddInternalCall("Shado.InternalCalls", #icall, (void*)InternalCalls::icall)

#ifdef HZ_DIST
	#define HZ_ICALL_VALIDATE_PARAM(param)
	#define HZ_ICALL_VALIDATE_PARAM_V(param, value)
#else
#define HZ_ICALL_VALIDATE_PARAM(param) { if (!(param)) { SHADO_CORE_ERROR("{} called with an invalid value ({}) for parameter '{}'", HZ_FUNCTION_NAME, param, #param); } }
#define HZ_ICALL_VALIDATE_PARAM_V(param, value) { if (!(param)) { SHADO_CORE_ERROR("{} called with an invalid value ({}) for parameter '{}'.", HZ_FUNCTION_NAME, value, #param); } }
#endif

    std::unordered_map<Coral::TypeId, std::function<void(Entity&)>> s_CreateComponentFuncs;
    std::unordered_map<Coral::TypeId, std::function<bool(Entity&)>> s_HasComponentFuncs;
    std::unordered_map<Coral::TypeId, std::function<void(Entity&)>> s_RemoveComponentFuncs;

    template <typename TComponent>
    static void RegisterManagedComponent(Coral::ManagedAssembly& coreAssembly) {
        // NOTE(Peter): Get the demangled type name of TComponent
        const TypeNameString& componentTypeName = TypeInfo<TComponent, true>().Name();
        std::string componentName = std::format("Shado.{}", componentTypeName);

        auto& type = coreAssembly.GetType(componentName);

        if (type) {
            s_CreateComponentFuncs[type.GetTypeId()] = [](Entity& entity) { entity.addComponent<TComponent>(); };
            s_HasComponentFuncs[type.GetTypeId()] = [](Entity& entity) { return entity.hasComponent<TComponent>(); };
            s_RemoveComponentFuncs[type.GetTypeId()] = [](Entity& entity) { entity.removeComponent<TComponent>(); };
        }
        else {
            SHADO_CORE_ASSERT(false, "No C# component class found for {}!", componentName);
        }
    }

    template <typename TComponent>
    static void RegisterManagedComponent(std::function<void(Entity&)>&& addFunction,
                                         Coral::ManagedAssembly& coreAssembly) {
        // NOTE(Peter): Get the demangled type name of TComponent
        const TypeNameString& componentTypeName = TypeInfo<TComponent, true>().Name();
        std::string componentName = std::format("Shado.{}", componentTypeName);

        auto& type = coreAssembly.GetType(componentName);

        if (type) {
            s_CreateComponentFuncs[type.GetTypeId()] = std::move(addFunction);
            s_HasComponentFuncs[type.GetTypeId()] = [](Entity& entity) { return entity.hasComponent<TComponent>(); };
            s_RemoveComponentFuncs[type.GetTypeId()] = [](Entity& entity) { entity.removeComponent<TComponent>(); };
        }
        else {
            SHADO_CORE_ASSERT(false, "No C# component class found for {}!", componentName);
        }
    }

    template <typename... Args>
    static void WarnWithTrace(std::format_string<Args...> format, Args&&... args) {
        /*auto stackTrace = ScriptUtils::GetCurrentStackTrace();
        std::string formattedMessage = std::format(inFormat, std::forward<TArgs>(inArgs)...);
        Log::GetEditorConsoleLogger()->warn("{}\nStack Trace: {}", formattedMessage, stackTrace);*/
    }

    template <typename... Args>
    static void ErrorWithTrace(const std::format_string<Args...> format, Args&&... args) {
        /*auto stackTrace = ScriptUtils::GetCurrentStackTrace();
        std::string formattedMessage = std::format(inFormat, std::forward<TArgs>(inArgs)...);
        Log::GetEditorConsoleLogger()->error("{}\nStack Trace: {}", formattedMessage, stackTrace);*/
    }

    void ScriptGlue::RegisterGlue(Coral::ManagedAssembly& coreAssembly) {
        if (!s_CreateComponentFuncs.empty()) {
            s_CreateComponentFuncs.clear();
            s_HasComponentFuncs.clear();
            s_RemoveComponentFuncs.clear();
        }

        RegisterComponentTypes(coreAssembly);
        RegisterInternalCalls(coreAssembly);

        coreAssembly.UploadInternalCalls();
    }

    void ScriptGlue::RegisterComponentTypes(Coral::ManagedAssembly& coreAssembly) {
        RegisterManagedComponent<TransformComponent>(coreAssembly);
        RegisterManagedComponent<TagComponent>(coreAssembly);
        RegisterManagedComponent<ScriptComponent>(coreAssembly);
        RegisterManagedComponent<CameraComponent>(coreAssembly);
        RegisterManagedComponent<SpriteRendererComponent>(coreAssembly);
        RegisterManagedComponent<CircleRendererComponent>(coreAssembly);
        RegisterManagedComponent<LineRendererComponent>(coreAssembly);
        RegisterManagedComponent<RigidBody2DComponent>(coreAssembly);
        RegisterManagedComponent<BoxCollider2DComponent>(coreAssembly);
        RegisterManagedComponent<CircleCollider2DComponent>(coreAssembly);
        RegisterManagedComponent<TextComponent>(coreAssembly);
    }

    void ScriptGlue::RegisterInternalCalls(Coral::ManagedAssembly& coreAssembly) {
        SHADO_ADD_INTERNAL_CALL(Application_Quit);
        SHADO_ADD_INTERNAL_CALL(Application_GetTime);
        SHADO_ADD_INTERNAL_CALL(Application_GetWidth);
        SHADO_ADD_INTERNAL_CALL(Application_GetHeight);
        SHADO_ADD_INTERNAL_CALL(Application_GetImGuiWindowSize);
        SHADO_ADD_INTERNAL_CALL(Application_IsImGuiWindowHovered);

        SHADO_ADD_INTERNAL_CALL(Window_GetPosition);
        SHADO_ADD_INTERNAL_CALL(Window_GetSize);
        SHADO_ADD_INTERNAL_CALL(Window_SetSize);
        SHADO_ADD_INTERNAL_CALL(Window_GetMode);
        SHADO_ADD_INTERNAL_CALL(Window_SetMode);
        SHADO_ADD_INTERNAL_CALL(Window_GetTitle);
        SHADO_ADD_INTERNAL_CALL(Window_SetTitle);
        SHADO_ADD_INTERNAL_CALL(Window_GetVSync);
        SHADO_ADD_INTERNAL_CALL(Window_SetVSync);
        SHADO_ADD_INTERNAL_CALL(Window_SetOpacity);

        SHADO_ADD_INTERNAL_CALL(Entity_GetParent);
        SHADO_ADD_INTERNAL_CALL(Entity_SetParent);
        SHADO_ADD_INTERNAL_CALL(Entity_GetChildren);
        SHADO_ADD_INTERNAL_CALL(Entity_CreateComponent);
        SHADO_ADD_INTERNAL_CALL(Entity_HasComponent);
        SHADO_ADD_INTERNAL_CALL(Entity_RemoveComponent);
        SHADO_ADD_INTERNAL_CALL(Entity_FindEntityByName);
        SHADO_ADD_INTERNAL_CALL(Entity_CreateEmptyEntity);
        SHADO_ADD_INTERNAL_CALL(Entity_Destroy);

        SHADO_ADD_INTERNAL_CALL(Prefab_Instantiate);

        SHADO_ADD_INTERNAL_CALL(Scene_IsEntityValid);
        SHADO_ADD_INTERNAL_CALL(Scene_LoadScene);
        SHADO_ADD_INTERNAL_CALL(Scene_GetAllEntities);

        SHADO_ADD_INTERNAL_CALL(TagComponent_GetTag);
        SHADO_ADD_INTERNAL_CALL(TagComponent_SetTag);

        SHADO_ADD_INTERNAL_CALL(TransformComponent_GetTransform);
        SHADO_ADD_INTERNAL_CALL(TransformComponent_SetTransform);

        SHADO_ADD_INTERNAL_CALL(ScriptComponent_GetInstance);

        SHADO_ADD_INTERNAL_CALL(CameraComponent_GetPrimary);
        SHADO_ADD_INTERNAL_CALL(CameraComponent_SetPrimary);
        SHADO_ADD_INTERNAL_CALL(CameraComponent_GetType);
        SHADO_ADD_INTERNAL_CALL(CameraComponent_SetType);
        SHADO_ADD_INTERNAL_CALL(CameraComponent_SetViewportSize);

        SHADO_ADD_INTERNAL_CALL(SpriteRendererComponent_GetColor);
        SHADO_ADD_INTERNAL_CALL(SpriteRendererComponent_SetColor);
        SHADO_ADD_INTERNAL_CALL(SpriteRendererComponent_GetTilingFactor);
        SHADO_ADD_INTERNAL_CALL(SpriteRendererComponent_SetTilingFactor);
        SHADO_ADD_INTERNAL_CALL(SpriteRendererComponent_GetTexture);
        SHADO_ADD_INTERNAL_CALL(SpriteRendererComponent_SetTexture);
        SHADO_ADD_INTERNAL_CALL(SpriteRendererComponent_GetShader);
        SHADO_ADD_INTERNAL_CALL(SpriteRendererComponent_SetShader);

        SHADO_ADD_INTERNAL_CALL(CircleRendererComponent_GetColor);
        SHADO_ADD_INTERNAL_CALL(CircleRendererComponent_SetColor);
        SHADO_ADD_INTERNAL_CALL(CircleRendererComponent_GetThickness);
        SHADO_ADD_INTERNAL_CALL(CircleRendererComponent_SetThickness);
        SHADO_ADD_INTERNAL_CALL(CircleRendererComponent_GetFade);
        SHADO_ADD_INTERNAL_CALL(CircleRendererComponent_SetFade);

        SHADO_ADD_INTERNAL_CALL(LineRendererComponent_GetTarget);
        SHADO_ADD_INTERNAL_CALL(LineRendererComponent_SetTarget);
        SHADO_ADD_INTERNAL_CALL(LineRendererComponent_GetColour);
        SHADO_ADD_INTERNAL_CALL(LineRendererComponent_SetColour);

        SHADO_ADD_INTERNAL_CALL(RigidBody2DComponent_GetBodyType);
        SHADO_ADD_INTERNAL_CALL(RigidBody2DComponent_SetBodyType);
        SHADO_ADD_INTERNAL_CALL(RigidBody2DComponent_GetLinearVelocity);
        SHADO_ADD_INTERNAL_CALL(Rigidbody2DComponent_ApplyLinearImpulse);
        SHADO_ADD_INTERNAL_CALL(Rigidbody2DComponent_ApplyLinearImpulseToCenter);

        SHADO_ADD_INTERNAL_CALL(BoxCollider2DComponent_GetOffset);
        SHADO_ADD_INTERNAL_CALL(BoxCollider2DComponent_SetOffset);
        SHADO_ADD_INTERNAL_CALL(BoxCollider2DComponent_GetSize);
        SHADO_ADD_INTERNAL_CALL(BoxCollider2DComponent_SetSize);
        SHADO_ADD_INTERNAL_CALL(BoxCollider2DComponent_GetDensity);
        SHADO_ADD_INTERNAL_CALL(BoxCollider2DComponent_SetDensity);
        SHADO_ADD_INTERNAL_CALL(BoxCollider2DComponent_GetFriction);
        SHADO_ADD_INTERNAL_CALL(BoxCollider2DComponent_SetFriction);
        SHADO_ADD_INTERNAL_CALL(BoxCollider2DComponent_GetRestitution);
        SHADO_ADD_INTERNAL_CALL(BoxCollider2DComponent_SetRestitution);
        SHADO_ADD_INTERNAL_CALL(BoxCollider2DComponent_GetRestitution);
        SHADO_ADD_INTERNAL_CALL(BoxCollider2DComponent_SetRestitution);
        SHADO_ADD_INTERNAL_CALL(BoxCollider2DComponent_GetRestitutionThreshold);
        SHADO_ADD_INTERNAL_CALL(BoxCollider2DComponent_SetRestitutionThreshold);

        SHADO_ADD_INTERNAL_CALL(TextComponent_GetText);
        SHADO_ADD_INTERNAL_CALL(TextComponent_SetText);
        SHADO_ADD_INTERNAL_CALL(TextComponent_GetColor);
        SHADO_ADD_INTERNAL_CALL(TextComponent_SetColor);
        SHADO_ADD_INTERNAL_CALL(TextComponent_GetLineSpacing);
        SHADO_ADD_INTERNAL_CALL(TextComponent_SetLineSpacing);
        SHADO_ADD_INTERNAL_CALL(TextComponent_GetKerning);
        SHADO_ADD_INTERNAL_CALL(TextComponent_SetKerning);

        //============================================================================================

        SHADO_ADD_INTERNAL_CALL(Texture2D_Create);

        SHADO_ADD_INTERNAL_CALL(Shader_Create);
        SHADO_ADD_INTERNAL_CALL(Shader_SetInt);
        SHADO_ADD_INTERNAL_CALL(Shader_SetFloat);
        SHADO_ADD_INTERNAL_CALL(Shader_SetFloat2);
        SHADO_ADD_INTERNAL_CALL(Shader_SetFloat3);
        SHADO_ADD_INTERNAL_CALL(Shader_SetFloat4);

        SHADO_ADD_INTERNAL_CALL(Physics2D_Raycast);

        SHADO_ADD_INTERNAL_CALL(Log_LogMessage);

        SHADO_ADD_INTERNAL_CALL(Input_IsKeyPressed);
        SHADO_ADD_INTERNAL_CALL(Input_IsMouseButtonPressed);
        SHADO_ADD_INTERNAL_CALL(Input_GetMousePosition);

        SHADO_ADD_INTERNAL_CALL(Project_GetAssetDirectory);
        SHADO_ADD_INTERNAL_CALL(Project_GetProjectDirectory);
        SHADO_ADD_INTERNAL_CALL(Project_GetRelativePath);
    }

    namespace InternalCalls {
        static inline Entity GetEntity(uint64_t entityID) {
            Ref<Scene> scene = ScriptEngine::GetInstance().GetCurrentScene();
            SHADO_CORE_ASSERT(scene, "No active scene!");
            return scene->getEntityById(entityID);
        };

#pragma region Application

        void Application_Quit() {
            Application::get().close();
        }

        float Application_GetTime() { return Application::get().getTime(); }
        uint32_t Application_GetWidth() { return ScriptEngine::GetInstance().GetCurrentScene()->getViewport().x; }
        uint32_t Application_GetHeight() { return ScriptEngine::GetInstance().GetCurrentScene()->getViewport().y; }

        void Application_GetImGuiWindowSize(Coral::String name, glm::vec2* outSize) {
            // Small hack to get around ImGui not having a way to get a given window size
            ImGuiContext* context = ImGui::GetCurrentContext();
            ImGuiWindow* currentWin = ImGui::GetCurrentWindow();
            auto* targetWin = ImGui::FindWindowByName(std::string(name).c_str());

            context->CurrentWindow = targetWin;
            *outSize = {ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y};

            context->CurrentWindow = currentWin;
        }

        bool Application_IsImGuiWindowHovered(Coral::String windowName) {
            ImGuiContext* context = ImGui::GetCurrentContext();
            auto* currentWin = ImGui::GetCurrentWindow();
            auto* targetWin = ImGui::FindWindowByName(std::string(windowName).c_str());

            context->CurrentWindow = targetWin;
            bool result = ImGui::IsWindowHovered();
            context->CurrentWindow = currentWin;

            return result;
        }

#pragma endregion

#pragma region Window
        void Window_GetPosition(glm::vec2* outPosition) {
            *outPosition = {0, 0};
            outPosition->x = Application::get().getWindow().getPosX();
            outPosition->y = Application::get().getWindow().getPosY();
        }

        void Window_GetSize(glm::vec2* outSize) {
            *outSize = {Application::get().getWindow().getWidth(), Application::get().getWindow().getHeight()};
        }

        void Window_SetSize(glm::vec2* inSize) {
            Application::get().getWindow().resize(inSize->x, inSize->y);
        }

        int Window_GetMode() {
            return (int)Application::get().getWindow().getMode();
        }

        void Window_SetMode(int mode) {
            Application::get().getWindow().setMode((WindowMode)mode);
        }

        Coral::String Window_GetTitle() {
            return Coral::String::New(Application::get().getWindow().getTitle().data());
        }

        void Window_SetTitle(Coral::String title) {
            Application::get().getWindow().setTitle(title);
        }

        bool Window_GetVSync() {
            return Application::get().getWindow().isVSync();
        }

        void Window_SetVSync(bool enabled) {
            Application::get().getWindow().setVSync(enabled);
        }

        void Window_SetOpacity(float opacity) {
            Application::get().getWindow().setOpacity(opacity);
        }
#pragma endregion

#pragma region Entity

        uint64_t Entity_GetParent(uint64_t entityID) {
            auto entity = GetEntity(entityID);
            auto scene = ScriptEngine::GetInstance().GetCurrentScene();
            HZ_ICALL_VALIDATE_PARAM_V(entity, entityID);
            return entity.getComponent<TransformComponent>().getParent(*scene).getUUID();
        }

        void Entity_SetParent(uint64_t entityID, uint64_t parentID) {
            Entity child = GetEntity(entityID);
            auto scene = ScriptEngine::GetInstance().GetCurrentScene();
            HZ_ICALL_VALIDATE_PARAM_V(child, entityID);

            if (parentID == 0) {
                child.getComponent<TransformComponent>().setParent(child, {});
            }
            else {
                Entity parent = GetEntity(parentID);
                HZ_ICALL_VALIDATE_PARAM_V(parent, parentID);
                child.getComponent<TransformComponent>().setParent(child, parent);
            }
        }

        Coral::Array<uint64_t> Entity_GetChildren(uint64_t entityID) {
            Entity entity = GetEntity(entityID);
            HZ_ICALL_VALIDATE_PARAM_V(entity, entityID);

            const auto& children = entity.getChildren();
            auto result = Coral::Array<uint64_t>::New(int32_t(children.size()));
            for (uint32_t i = 0; i < children.size(); i++)
                result[i] = children[i].getUUID();

            return result;
        }

        void Entity_CreateComponent(uint64_t entityID, Coral::ReflectionType componentType) {
            auto entity = GetEntity(entityID);
            HZ_ICALL_VALIDATE_PARAM_V(entity, entityID);

            if (!entity)
                return;

            Coral::Type& type = componentType;

            if (!type)
                return;

            //Coral::ScopedString typeName = type.GetFullName();

            if (auto it = s_HasComponentFuncs.find(type.GetTypeId()); it != s_HasComponentFuncs.end() && it->
                second(entity)) {
                //WarnWithTrace("Attempting to add duplicate component '{}' to entity '{}', ignoring.", std::string(typeName), entity.Name());
                return;
            }

            if (auto it = s_CreateComponentFuncs.find(type.GetTypeId()); it != s_CreateComponentFuncs.end()) {
                return it->second(entity);
            }

            //ErrorWithTrace("Cannot create component of type '{}' for entity '{}'. That component hasn't been registered with the engine.", std::string(typeName), entity.Name());
        }

        bool Entity_HasComponent(uint64_t entityID, Coral::ReflectionType componentType) {
            auto entity = GetEntity(entityID);
            HZ_ICALL_VALIDATE_PARAM_V(entity, entityID);

            if (!entity)
                return false;

            Coral::Type& type = componentType;

            if (!type)
                return false;

            //Coral::ScopedString typeName = type.GetFullName();

            if (auto it = s_HasComponentFuncs.find(type.GetTypeId()); it != s_HasComponentFuncs.end()) {
                // Note (0x): compiler bug?  if you return it->second(entity) directly, it does not return the correct result
                // e.g. evaluating the function returns false, but then the caller receives true ??!!
                // Copying the function first, then calling it works as expected.
                auto func = it->second;
                return func(entity);
            }

            //ErrorWithTrace("Cannot check if entity '{}' has a component of type '{}'. That component hasn't been registered with the engine.", entity.Name(), std::string(typeName));
            return false;
        }

        bool Entity_RemoveComponent(uint64_t entityID, Coral::ReflectionType componentType) {
            auto entity = GetEntity(entityID);
            HZ_ICALL_VALIDATE_PARAM_V(entity, entityID);

            if (!entity)
                return false;

            Coral::Type& type = componentType;

            if (!type)
                return false;

            //Coral::ScopedString typeName = type.GetFullName();

            if (auto it = s_HasComponentFuncs.find(type.GetTypeId()); it == s_HasComponentFuncs.end() || !it->
                second(entity)) {
                //WarnWithTrace("Tried to remove component '{}' from entity '{}' even though it doesn't have that component.", std::string(typeName), entity.Name());
                return false;
            }

            if (auto it = s_RemoveComponentFuncs.find(type.GetTypeId()); it != s_RemoveComponentFuncs.end()) {
                it->second(entity);
                return true;
            }

            //ErrorWithTrace("Cannot remove component of type '{}' from entity '{}'. That component hasn't been registered with the engine.", std::string(typeName), entity.Name());
            return false;
        }

        uint64_t Entity_FindEntityByName(Coral::String name) {
            auto scene = ScriptEngine::GetInstance().GetCurrentScene();
            std::string nameStr = name;
            Entity entity = scene->findEntityByName(nameStr);
            if (entity.isValid())
                return entity.getUUID();
            else
                return 0;
        }

        uint64_t Entity_CreateEmptyEntity() {
            auto scene = ScriptEngine::GetInstance().GetCurrentScene();
            auto entity = scene->createEntity();
            return entity.getUUID();
        }
        
        void Entity_Destroy(uint64_t entityID) {
            auto scene = ScriptEngine::GetInstance().GetCurrentScene();
            auto entity = GetEntity(entityID);
            HZ_ICALL_VALIDATE_PARAM_V(entity, entityID);

            scene->destroyEntity(entity);
        }

#pragma endregion

#pragma region Prefab

        uint64_t Prefab_Instantiate(uint64_t prefabID, glm::vec3 position) {
            auto scene = ScriptEngine::GetInstance().GetCurrentScene();
            auto prefab = Prefab::GetPrefabById(prefabID);
            if (!prefab || !scene)
                return 0;

            Entity e = scene->instantiatePrefab(prefab);
            e.getComponent<TransformComponent>().position = position;
            return e.getUUID();
        }

#pragma endregion

#pragma region Scene
        bool Scene_IsEntityValid(uint64_t entityID) {
            auto scene = ScriptEngine::GetInstance().GetCurrentScene();
            return scene->getEntityById(entityID).isValid();
        }

        Coral::String Scene_LoadScene(Coral::String scenePath) {
            // Search for the scene in the project directory
            std::string sceneNameStr = scenePath;
            const std::filesystem::path projectPath = Project::GetProjectDirectory();

            // Search for the scene in the project directory recursively
            std::filesystem::path sceneFullPath;
            for (const auto& entry : std::filesystem::recursive_directory_iterator(projectPath)) {
                if (entry.is_regular_file() && entry.path().filename().string().find(sceneNameStr) !=
                    std::string::npos) {
                    sceneFullPath = entry.path();
                    break;
                }
            }

            // New scene
            Application::dispatchEvent(SceneChangedEvent(sceneFullPath));

            return Coral::String::New(sceneFullPath.string());
        }

        Coral::Array<uint64_t> Scene_GetAllEntities() {
            std::vector<uint64_t> entityIDs;
            auto scene = ScriptEngine::GetInstance().GetCurrentScene();
            for (auto& entity : scene->getAllEntities()) {
                entityIDs.push_back(entity.getUUID());
            }
            return Coral::Array<uint64_t>::New(entityIDs);
        }
#pragma endregion

#pragma region TagComponent

        Coral::String TagComponent_GetTag(uint64_t entityID) {
            auto entity = GetEntity(entityID);
            HZ_ICALL_VALIDATE_PARAM_V(entity, entityID);

            const auto& tagComponent = entity.getComponent<TagComponent>();
            return Coral::String::New(tagComponent.tag);
        }

        void TagComponent_SetTag(uint64_t entityID, Coral::String inTag) {
            auto entity = GetEntity(entityID);
            HZ_ICALL_VALIDATE_PARAM_V(entity, entityID);
            auto& tagComponent = entity.getComponent<TagComponent>();

            std::string tag = inTag;
            tagComponent.tag = tag;
        }

#pragma endregion

#pragma region TransformComponent

        void TransformComponent_GetTransform(uint64_t entityID, Transform* outTransform) {
            auto entity = GetEntity(entityID);
            HZ_ICALL_VALIDATE_PARAM_V(entity, entityID);

            const auto& tc = entity.getComponent<TransformComponent>();
            outTransform->Translation = tc.position;
            outTransform->Rotation = tc.rotation;
            outTransform->Scale = tc.scale;
        }


        void TransformComponent_SetTransform(uint64_t entityID, Transform* inTransform) {
            auto entity = GetEntity(entityID);
            HZ_ICALL_VALIDATE_PARAM_V(entity, entityID);


            if (inTransform == nullptr) {
                ErrorWithTrace("Attempting to set a null transform for entity '{}'",
                               entity.getComponent<TagComponent>().tag);
                return;
            }

            auto& tc = entity.getComponent<TransformComponent>();
            // call TransformComponent_Set... methods so that SetTransform(transform) behaves the same way as
            // setting individual components.
            tc.position = inTransform->Translation;
            tc.rotation = inTransform->Rotation;
            tc.scale = inTransform->Scale;
        }

#pragma endregion

#pragma region ScriptComponent

        Coral::ManagedObject ScriptComponent_GetInstance(uint64_t entityID) {
            auto entity = GetEntity(entityID);
            HZ_ICALL_VALIDATE_PARAM_V(entity, entityID);
            //HZ_ICALL_VALIDATE_PARAM(entity.HasComponent<ScriptComponent>());
            if (!entity.hasComponent<ScriptComponent>()) {
                SHADO_CORE_ERROR("ScriptComponent_GetInstance no ScriptComponent apparently, returning null");
                return Coral::ManagedObject();
            }

            const auto& component = entity.getComponent<ScriptComponent>();
#if 0
			if (!ScriptEngine::IsModuleValid(component.ScriptClassHandle))
			{
				ErrorWithTrace("Entity is referencing an invalid C# class!");
				return nullptr;
			}

			if (!ScriptEngine::IsEntityInstantiated(entity))
			{
				// Check if the entity is instantiated WITHOUT checking if the OnCreate method has run
				if (ScriptEngine::IsEntityInstantiated(entity, false))
				{
					// If so, call OnCreate here...
					ScriptEngine::CallMethod(component.ManagedInstance, "OnCreate");

					// NOTE(Peter): Don't use scriptComponent as a reference and modify it here
					//				If OnCreate spawns a lot of entities we would loose our reference
					//				to the script component...
					entity.GetComponent<ScriptComponent>().IsRuntimeInitialized = true;

					return GCManager::GetReferencedObject(component.ManagedInstance);
				}
				else if (component.ManagedInstance == nullptr)
				{
					ScriptEngine::RuntimeInitializeScriptEntity(entity);
					return GCManager::GetReferencedObject(component.ManagedInstance);
				}

				ErrorWithTrace("Entity '{0}' isn't instantiated?", entity.Name());
				return nullptr;
			}
#endif

            if (!component.Instance.IsValid()) {
                SHADO_CORE_ERROR("ScriptComponent_GetInstance returning null managed object");
                return Coral::ManagedObject();
            }

            SHADO_CORE_ASSERT(component.Instance.IsValid(), "");
            return *component.Instance.GetHandle();
        }

#pragma endregion

#pragma region CameraComponent

        bool CameraComponent_GetPrimary(uint64_t entityID) {
            auto entity = GetEntity(entityID);
            HZ_ICALL_VALIDATE_PARAM_V(entity, entityID);
            HZ_ICALL_VALIDATE_PARAM(entity.hasComponent<CameraComponent>());
            return entity.getComponent<CameraComponent>().primary;
        }

        void CameraComponent_SetPrimary(uint64_t entityID, bool inPrimary) {
            auto entity = GetEntity(entityID);
            HZ_ICALL_VALIDATE_PARAM_V(entity, entityID);
            HZ_ICALL_VALIDATE_PARAM(entity.hasComponent<CameraComponent>());
            entity.getComponent<CameraComponent>().primary = inPrimary;
        }

        CameraComponent::Type CameraComponent_GetType(uint64_t entityID) {
            auto entity = GetEntity(entityID);
            HZ_ICALL_VALIDATE_PARAM_V(entity, entityID);
            HZ_ICALL_VALIDATE_PARAM(entity.hasComponent<CameraComponent>());
            return entity.getComponent<CameraComponent>().type;
        }

        void CameraComponent_SetType(uint64_t entityID, CameraComponent::Type inType) {
            auto entity = GetEntity(entityID);
            HZ_ICALL_VALIDATE_PARAM_V(entity, entityID);
            HZ_ICALL_VALIDATE_PARAM(entity.hasComponent<CameraComponent>());
            entity.getComponent<CameraComponent>().type = inType;
        }

        void CameraComponent_SetViewportSize(uint64_t entityID, uint32_t inWidth, uint32_t inHeight) {
            auto entity = GetEntity(entityID);
            HZ_ICALL_VALIDATE_PARAM_V(entity, entityID);
            HZ_ICALL_VALIDATE_PARAM(entity.hasComponent<CameraComponent>());
            entity.getComponent<CameraComponent>().setViewportSize(inWidth, inHeight);
        }

#pragma endregion

#pragma region SpriteRendererComponent

        void SpriteRendererComponent_GetColor(uint64_t entityID, glm::vec4* outColor) {
            auto entity = GetEntity(entityID);
            HZ_ICALL_VALIDATE_PARAM_V(entity, entityID);
            HZ_ICALL_VALIDATE_PARAM(entity.hasComponent<SpriteRendererComponent>());
            *outColor = entity.getComponent<SpriteRendererComponent>().color;
        }

        void SpriteRendererComponent_SetColor(uint64_t entityID, glm::vec4* inColor) {
            auto entity = GetEntity(entityID);
            HZ_ICALL_VALIDATE_PARAM_V(entity, entityID);
            HZ_ICALL_VALIDATE_PARAM(entity.hasComponent<SpriteRendererComponent>());
            entity.getComponent<SpriteRendererComponent>().color = *inColor;
        }

        float SpriteRendererComponent_GetTilingFactor(uint64_t entityID) {
            auto entity = GetEntity(entityID);
            HZ_ICALL_VALIDATE_PARAM_V(entity, entityID);
            HZ_ICALL_VALIDATE_PARAM(entity.hasComponent<SpriteRendererComponent>());
            return entity.getComponent<SpriteRendererComponent>().tilingFactor;
        }

        void SpriteRendererComponent_SetTilingFactor(uint64_t entityID, float tilingFactor) {
            auto entity = GetEntity(entityID);
            HZ_ICALL_VALIDATE_PARAM_V(entity, entityID);
            HZ_ICALL_VALIDATE_PARAM(entity.hasComponent<SpriteRendererComponent>());
            entity.getComponent<SpriteRendererComponent>().tilingFactor = tilingFactor;
        }

        uint64_t SpriteRendererComponent_GetTexture(uint64_t entityID) {
            auto entity = GetEntity(entityID);
            HZ_ICALL_VALIDATE_PARAM_V(entity, entityID);
            HZ_ICALL_VALIDATE_PARAM(entity.hasComponent<SpriteRendererComponent>());
            return entity.getComponent<SpriteRendererComponent>().texture;
        }

        void SpriteRendererComponent_SetTexture(uint64_t entityID, AssetHandle textureHandle) {
            auto entity = GetEntity(entityID);
            HZ_ICALL_VALIDATE_PARAM_V(entity, entityID);
            HZ_ICALL_VALIDATE_PARAM(entity.hasComponent<SpriteRendererComponent>());

            entity.getComponent<SpriteRendererComponent>().texture = textureHandle;
        }

        uint64_t SpriteRendererComponent_GetShader(uint64_t entityID) {
            auto entity = GetEntity(entityID);
            HZ_ICALL_VALIDATE_PARAM_V(entity, entityID);
            HZ_ICALL_VALIDATE_PARAM(entity.hasComponent<SpriteRendererComponent>());
            return entity.getComponent<SpriteRendererComponent>().texture;
        }

        void SpriteRendererComponent_SetShader(uint64_t entityID, AssetHandle inShader) {
            auto entity = GetEntity(entityID);
            HZ_ICALL_VALIDATE_PARAM_V(entity, entityID);
            HZ_ICALL_VALIDATE_PARAM(entity.hasComponent<SpriteRendererComponent>());

            entity.getComponent<SpriteRendererComponent>().shader = inShader;
        }

#pragma endregion

#pragma region CircleRendererComponent
        void CircleRendererComponent_GetColor(uint64_t entityID, glm::vec4* outColor) {
            auto entity = GetEntity(entityID);
            HZ_ICALL_VALIDATE_PARAM_V(entity, entityID);
            HZ_ICALL_VALIDATE_PARAM(entity.hasComponent<CircleRendererComponent>());
            *outColor = entity.getComponent<CircleRendererComponent>().color;
        }

        void CircleRendererComponent_SetColor(uint64_t entityID, glm::vec4* inColor) {
            auto entity = GetEntity(entityID);
            HZ_ICALL_VALIDATE_PARAM_V(entity, entityID);
            HZ_ICALL_VALIDATE_PARAM(entity.hasComponent<CircleRendererComponent>());
            entity.getComponent<CircleRendererComponent>().color = *inColor;
        }

        float CircleRendererComponent_GetThickness(uint64_t entityID) {
            auto entity = GetEntity(entityID);
            HZ_ICALL_VALIDATE_PARAM_V(entity, entityID);
            HZ_ICALL_VALIDATE_PARAM(entity.hasComponent<CircleRendererComponent>());
            return entity.getComponent<CircleRendererComponent>().thickness;
        }

        void CircleRendererComponent_SetThickness(uint64_t entityID, float inThickness) {
            auto entity = GetEntity(entityID);
            HZ_ICALL_VALIDATE_PARAM_V(entity, entityID);
            HZ_ICALL_VALIDATE_PARAM(entity.hasComponent<CircleRendererComponent>());
            entity.getComponent<CircleRendererComponent>().thickness = inThickness;
        }

        float CircleRendererComponent_GetFade(uint64_t entityID) {
            auto entity = GetEntity(entityID);
            HZ_ICALL_VALIDATE_PARAM_V(entity, entityID);
            HZ_ICALL_VALIDATE_PARAM(entity.hasComponent<CircleRendererComponent>());
            return entity.getComponent<CircleRendererComponent>().fade;
        }

        void CircleRendererComponent_SetFade(uint64_t entityID, float inFade) {
            auto entity = GetEntity(entityID);
            HZ_ICALL_VALIDATE_PARAM_V(entity, entityID);
            HZ_ICALL_VALIDATE_PARAM(entity.hasComponent<CircleRendererComponent>());
            entity.getComponent<CircleRendererComponent>().fade = inFade;
        }

#pragma endregion

#pragma region LineRendererComponent
        void LineRendererComponent_GetTarget(uint64_t entityID, glm::vec3* outTarget) {
            auto entity = GetEntity(entityID);
            HZ_ICALL_VALIDATE_PARAM_V(entity, entityID);
            HZ_ICALL_VALIDATE_PARAM(entity.hasComponent<LineRendererComponent>());
            *outTarget = entity.getComponent<LineRendererComponent>().target;
        }

        void LineRendererComponent_SetTarget(uint64_t entityID, glm::vec3* inTarget) {
            auto entity = GetEntity(entityID);
            HZ_ICALL_VALIDATE_PARAM_V(entity, entityID);
            HZ_ICALL_VALIDATE_PARAM(entity.hasComponent<LineRendererComponent>());
            entity.getComponent<LineRendererComponent>().target = *inTarget;
        }

        void LineRendererComponent_GetColour(uint64_t entityID, glm::vec4* outColor) {
            auto entity = GetEntity(entityID);
            HZ_ICALL_VALIDATE_PARAM_V(entity, entityID);
            HZ_ICALL_VALIDATE_PARAM(entity.hasComponent<LineRendererComponent>());
            *outColor = entity.getComponent<LineRendererComponent>().color;
        }

        void LineRendererComponent_SetColour(uint64_t entityID, glm::vec4* inColor) {
            auto entity = GetEntity(entityID);
            HZ_ICALL_VALIDATE_PARAM_V(entity, entityID);
            HZ_ICALL_VALIDATE_PARAM(entity.hasComponent<LineRendererComponent>());
            entity.getComponent<LineRendererComponent>().color = *inColor;
        }

#pragma endregion

#pragma region RigidBody2DComponent

        void RigidBody2DComponent_GetLinearVelocity(uint64_t entityID, glm::vec2* outVelocity) {
            auto entity = GetEntity(entityID);
            HZ_ICALL_VALIDATE_PARAM_V(entity, entityID);
            HZ_ICALL_VALIDATE_PARAM(entity.hasComponent<RigidBody2DComponent>());
            auto& rb2d = entity.getComponent<RigidBody2DComponent>();
            b2Body* body = (b2Body*)rb2d.runtimeBody;
            const b2Vec2& linearVelocity = body->GetLinearVelocity();
            *outVelocity = glm::vec2(linearVelocity.x, linearVelocity.y);
        }

        RigidBody2DComponent::BodyType RigidBody2DComponent_GetBodyType(uint64_t entityID) {
            auto entity = GetEntity(entityID);
            HZ_ICALL_VALIDATE_PARAM_V(entity, entityID);
            HZ_ICALL_VALIDATE_PARAM(entity.hasComponent<RigidBody2DComponent>());
            return entity.getComponent<RigidBody2DComponent>().type;
        }

        void RigidBody2DComponent_SetBodyType(uint64_t entityID, RigidBody2DComponent::BodyType inType) {
            auto entity = GetEntity(entityID);
            HZ_ICALL_VALIDATE_PARAM_V(entity, entityID);
            HZ_ICALL_VALIDATE_PARAM(entity.hasComponent<RigidBody2DComponent>());
            entity.getComponent<RigidBody2DComponent>().type = inType;
        }

        void Rigidbody2DComponent_ApplyLinearImpulse(uint64_t entityID, glm::vec2 impulse, glm::vec2 worldPosition,
                                                     bool wake) {
            auto entity = GetEntity(entityID);
            HZ_ICALL_VALIDATE_PARAM_V(entity, entityID);
            HZ_ICALL_VALIDATE_PARAM(entity.hasComponent<RigidBody2DComponent>());
            auto& rb2d = entity.getComponent<RigidBody2DComponent>();
            b2Body* body = (b2Body*)rb2d.runtimeBody;
            if (!body) {
                SHADO_CORE_WARN("Cannot call physics related function in OnCreate. Physics body is nullptr");
                return;
            }
            body->ApplyLinearImpulse(b2Vec2(impulse.x, impulse.y), b2Vec2(worldPosition.x, worldPosition.y), wake);
        }

        void Rigidbody2DComponent_ApplyLinearImpulseToCenter(uint64_t entityID, glm::vec2 impulse, bool wake) {
            auto entity = GetEntity(entityID);
            HZ_ICALL_VALIDATE_PARAM_V(entity, entityID);
            HZ_ICALL_VALIDATE_PARAM(entity.hasComponent<RigidBody2DComponent>());
            auto& rb2d = entity.getComponent<RigidBody2DComponent>();
            b2Body* body = (b2Body*)rb2d.runtimeBody;
            if (!body) {
                SHADO_CORE_WARN("Cannot call physics related function in OnCreate. Physics body is nullptr");
                return;
            }
            body->ApplyLinearImpulseToCenter(b2Vec2(impulse.x, impulse.y), wake);
        }

#pragma endregion

#pragma region BoxCollider2DComponent and CircleCollider2DComponent

        void BoxCollider2DComponent_GetOffset(uint64_t entityID, glm::vec2* outOffset) {
            auto entity = GetEntity(entityID);
            HZ_ICALL_VALIDATE_PARAM_V(entity, entityID);

            if (entity.hasComponent<BoxCollider2DComponent>()) {
                auto& bc = entity.getComponent<BoxCollider2DComponent>();
                *outOffset = bc.offset;
            }
            else if (entity.hasComponent<CircleCollider2DComponent>()) {
                auto& bc = entity.getComponent<CircleCollider2DComponent>();
                *outOffset = bc.offset;
            }
        }

        void BoxCollider2DComponent_SetOffset(uint64_t entityID, glm::vec2* inOffset) {
            auto entity = GetEntity(entityID);
            HZ_ICALL_VALIDATE_PARAM_V(entity, entityID);
            if (entity.hasComponent<BoxCollider2DComponent>()) {
                auto& bc = entity.getComponent<BoxCollider2DComponent>();
                bc.offset = *inOffset;
            }
            else if (entity.hasComponent<CircleCollider2DComponent>()) {
                auto& bc = entity.getComponent<CircleCollider2DComponent>();
                bc.offset = *inOffset;
            }
        }

        void BoxCollider2DComponent_GetSize(uint64_t entityID, glm::vec2* outSize) {
            auto entity = GetEntity(entityID);
            HZ_ICALL_VALIDATE_PARAM_V(entity, entityID);
            if (entity.hasComponent<BoxCollider2DComponent>()) {
                auto& bc = entity.getComponent<BoxCollider2DComponent>();
                *outSize = bc.size;
            }
            else if (entity.hasComponent<CircleCollider2DComponent>()) {
                auto& bc = entity.getComponent<CircleCollider2DComponent>();
                *outSize = bc.radius;
            }
        }

        void BoxCollider2DComponent_SetSize(uint64_t entityID, glm::vec2* inSize) {
            auto entity = GetEntity(entityID);
            HZ_ICALL_VALIDATE_PARAM_V(entity, entityID);
            if (entity.hasComponent<BoxCollider2DComponent>()) {
                auto& bc = entity.getComponent<BoxCollider2DComponent>();
                bc.size = *inSize;
            }
            else if (entity.hasComponent<CircleCollider2DComponent>()) {
                auto& bc = entity.getComponent<CircleCollider2DComponent>();
                bc.radius = *inSize;
            }
        }

        float BoxCollider2DComponent_GetDensity(uint64_t entityID) {
            auto entity = GetEntity(entityID);
            HZ_ICALL_VALIDATE_PARAM_V(entity, entityID);
            if (entity.hasComponent<BoxCollider2DComponent>()) {
                auto& bc = entity.getComponent<BoxCollider2DComponent>();
                return bc.density;
            }
            else if (entity.hasComponent<CircleCollider2DComponent>()) {
                auto& bc = entity.getComponent<CircleCollider2DComponent>();
                return bc.density;
            }
            return 0.0f;
        }

        void BoxCollider2DComponent_SetDensity(uint64_t entityID, float inDensity) {
            auto entity = GetEntity(entityID);
            HZ_ICALL_VALIDATE_PARAM_V(entity, entityID);
            if (entity.hasComponent<BoxCollider2DComponent>()) {
                auto& bc = entity.getComponent<BoxCollider2DComponent>();
                bc.density = inDensity;
            }
            else if (entity.hasComponent<CircleCollider2DComponent>()) {
                auto& bc = entity.getComponent<CircleCollider2DComponent>();
                bc.density = inDensity;
            }
        }

        float BoxCollider2DComponent_GetFriction(uint64_t entityID) {
            auto entity = GetEntity(entityID);
            HZ_ICALL_VALIDATE_PARAM_V(entity, entityID);
            if (entity.hasComponent<BoxCollider2DComponent>()) {
                auto& bc = entity.getComponent<BoxCollider2DComponent>();
                return bc.friction;
            }
            else if (entity.hasComponent<CircleCollider2DComponent>()) {
                auto& bc = entity.getComponent<CircleCollider2DComponent>();
                return bc.friction;
            }
            return 0.0f;
        }

        void BoxCollider2DComponent_SetFriction(uint64_t entityID, float inFriction) {
            auto entity = GetEntity(entityID);
            HZ_ICALL_VALIDATE_PARAM_V(entity, entityID);
            if (entity.hasComponent<BoxCollider2DComponent>()) {
                auto& bc = entity.getComponent<BoxCollider2DComponent>();
                bc.friction = inFriction;
            }
            else if (entity.hasComponent<CircleCollider2DComponent>()) {
                auto& bc = entity.getComponent<CircleCollider2DComponent>();
                bc.friction = inFriction;
            }
        }

        float BoxCollider2DComponent_GetRestitution(uint64_t entityID) {
            auto entity = GetEntity(entityID);
            HZ_ICALL_VALIDATE_PARAM_V(entity, entityID);
            if (entity.hasComponent<BoxCollider2DComponent>()) {
                auto& bc = entity.getComponent<BoxCollider2DComponent>();
                return bc.restitution;
            }
            else if (entity.hasComponent<CircleCollider2DComponent>()) {
                auto& bc = entity.getComponent<CircleCollider2DComponent>();
                return bc.restitution;
            }
            return 0.0f;
        }

        void BoxCollider2DComponent_SetRestitution(uint64_t entityID, float inRestitution) {
            auto entity = GetEntity(entityID);
            HZ_ICALL_VALIDATE_PARAM_V(entity, entityID);
            if (entity.hasComponent<BoxCollider2DComponent>()) {
                auto& bc = entity.getComponent<BoxCollider2DComponent>();
                bc.restitution = inRestitution;
            }
            else if (entity.hasComponent<CircleCollider2DComponent>()) {
                auto& bc = entity.getComponent<CircleCollider2DComponent>();
                bc.restitution = inRestitution;
            }
        }

        float BoxCollider2DComponent_GetRestitutionThreshold(uint64_t entityID) {
            auto entity = GetEntity(entityID);
            HZ_ICALL_VALIDATE_PARAM_V(entity, entityID);
            if (entity.hasComponent<BoxCollider2DComponent>()) {
                auto& bc = entity.getComponent<BoxCollider2DComponent>();
                return bc.restitutionThreshold;
            }
            else if (entity.hasComponent<CircleCollider2DComponent>()) {
                auto& bc = entity.getComponent<CircleCollider2DComponent>();
                return bc.restitutionThreshold;
            }
            return 0.0f;
        }

        void BoxCollider2DComponent_SetRestitutionThreshold(uint64_t entityID, float inRestitutionThreshold) {
            auto entity = GetEntity(entityID);
            HZ_ICALL_VALIDATE_PARAM_V(entity, entityID);
            if (entity.hasComponent<BoxCollider2DComponent>()) {
                auto& bc = entity.getComponent<BoxCollider2DComponent>();
                bc.restitutionThreshold = inRestitutionThreshold;
            }
            else if (entity.hasComponent<CircleCollider2DComponent>()) {
                auto& bc = entity.getComponent<CircleCollider2DComponent>();
                bc.restitutionThreshold = inRestitutionThreshold;
            }
        }
#pragma endregion

#pragma region TextComponent

        Coral::String TextComponent_GetText(uint64_t entityID) {
            auto entity = GetEntity(entityID);
            HZ_ICALL_VALIDATE_PARAM_V(entity, entityID);
            HZ_ICALL_VALIDATE_PARAM(entity.hasComponent<TextComponent>());

            const auto& component = entity.getComponent<TextComponent>();
            return Coral::String::New(component.text);
        }

        void TextComponent_SetText(uint64_t entityID, Coral::String text) {
            auto entity = GetEntity(entityID);
            HZ_ICALL_VALIDATE_PARAM_V(entity, entityID);
            HZ_ICALL_VALIDATE_PARAM(entity.hasComponent<TextComponent>());

            auto& component = entity.getComponent<TextComponent>();
            std::string cppText = text;
            component.text = cppText;
        }

        void TextComponent_GetColor(uint64_t entityID, glm::vec4* outColor) {
            auto entity = GetEntity(entityID);
            HZ_ICALL_VALIDATE_PARAM_V(entity, entityID);
            HZ_ICALL_VALIDATE_PARAM(entity.hasComponent<TextComponent>());

            const auto& component = entity.getComponent<TextComponent>();
            *outColor = component.color;
        }

        void TextComponent_SetColor(uint64_t entityID, glm::vec4* inColor) {
            auto entity = GetEntity(entityID);
            HZ_ICALL_VALIDATE_PARAM_V(entity, entityID);
            HZ_ICALL_VALIDATE_PARAM(entity.hasComponent<TextComponent>());

            auto& component = entity.getComponent<TextComponent>();
            component.color = *inColor;
        }

        float TextComponent_GetLineSpacing(uint64_t entityID) {
            auto entity = GetEntity(entityID);
            HZ_ICALL_VALIDATE_PARAM_V(entity, entityID);
            HZ_ICALL_VALIDATE_PARAM(entity.hasComponent<TextComponent>());
            auto& component = entity.getComponent<TextComponent>();
            return component.lineSpacing;
        }

        void TextComponent_SetLineSpacing(uint64_t entityID, float inLineSpacing) {
            auto entity = GetEntity(entityID);
            HZ_ICALL_VALIDATE_PARAM_V(entity, entityID);
            HZ_ICALL_VALIDATE_PARAM(entity.hasComponent<TextComponent>());
            auto& component = entity.getComponent<TextComponent>();
            component.lineSpacing = inLineSpacing;
        }

        float TextComponent_GetKerning(uint64_t entityID) {
            auto entity = GetEntity(entityID);
            HZ_ICALL_VALIDATE_PARAM_V(entity, entityID);
            HZ_ICALL_VALIDATE_PARAM(entity.hasComponent<TextComponent>());
            auto& component = entity.getComponent<TextComponent>();
            return component.kerning;
        }

        void TextComponent_SetKerning(uint64_t entityID, float inKerning) {
            auto entity = GetEntity(entityID);
            HZ_ICALL_VALIDATE_PARAM_V(entity, entityID);
            HZ_ICALL_VALIDATE_PARAM(entity.hasComponent<TextComponent>());
            auto& component = entity.getComponent<TextComponent>();
            component.kerning = inKerning;
        }

#pragma endregion

#pragma region Texture2D

        uint64_t Texture2D_Create(Coral::String inPath) {
            // See if the texture is in registry
            if (!Project::GetActive()->GetAssetManager()->IsPathInRegistry(std::string(inPath))) {
                AssetHandle handle = Project::GetActive()->GetEditorAssetManager()->ImportAsset(std::string(inPath), false);
                SHADO_CORE_ASSERT(Project::GetActive()->GetAssetManager()->IsAssetLoaded(handle), "Failed to load asset {}", std::string(inPath));
                return handle;
            }
            return Project::GetActive()->GetAssetManager()->GetHandleFromPath(std::string(inPath));
        }
#pragma endregion

#pragma region Shader
        uint64_t Shader_Create(Coral::String pathRelativeToProject) {
            // See if the texture is in registry
            if (!Project::GetActive()->GetAssetManager()->IsPathInRegistry(std::string(pathRelativeToProject))) {
                return Project::GetActive()->GetEditorAssetManager()->ImportAsset(std::string(pathRelativeToProject));
            }
            return Project::GetActive()->GetAssetManager()->GetHandleFromPath(std::string(pathRelativeToProject));
        }

        void Shader_SetInt(uint64_t inHandle, Coral::String inName, int inValue) {
            Ref<Shader> shader = AssetManager::GetAsset<Shader>(inHandle);
            if (shader)
                shader->setInt(std::string(inName), inValue);
        }

        void Shader_SetFloat(uint64_t inHandle, Coral::String inName, float inValue) {
            Ref<Shader> shader = AssetManager::GetAsset<Shader>(inHandle);
            if (shader)
                shader->setFloat(std::string(inName), inValue);
        }

        void Shader_SetFloat2(uint64_t inHandle, Coral::String inName, glm::vec2 inValue) {
            Ref<Shader> shader = AssetManager::GetAsset<Shader>(inHandle);
            if (shader)
                shader->setFloat2(std::string(inName), inValue);
        }

        void Shader_SetFloat3(uint64_t inHandle, Coral::String inName, glm::vec3 inValue) {
            Ref<Shader> shader = AssetManager::GetAsset<Shader>(inHandle);
            if (shader)
                shader->setFloat3(std::string(inName), inValue);
        }

        void Shader_SetFloat4(uint64_t inHandle, Coral::String inName, glm::vec4 inValue) {
            Ref<Shader> shader = AssetManager::GetAsset<Shader>(inHandle);
            if (shader)
                shader->setFloat4(std::string(inName), inValue);
        }

#pragma endregion

#pragma region Physics2D

        Coral::Array<ScriptRaycastHit2D> Physics2D_Raycast(RaycastData2D* inRaycastData) {
            auto scene = ScriptEngine::GetInstance().GetCurrentScene();
            HZ_ICALL_VALIDATE_PARAM_V(scene, false);

            struct Raycast2DResult {
                Entity HitEntity;
                glm::vec2 Point;
                glm::vec2 Normal;
                float Distance; // World units

                Raycast2DResult(Entity entity, glm::vec2 point, glm::vec2 normal, float distance)
                    : HitEntity(entity), Point(point), Normal(normal), Distance(distance) {
                }
            };

            class RayCastCallback : public b2RayCastCallback {
            public:
                RayCastCallback(Ref<Scene> scene, std::vector<Raycast2DResult>& results, glm::vec2 p0, glm::vec2 p1)
                    : m_Scene(scene), m_Results(results), m_Point0(p0), m_Point1(p1) {
                }

                virtual float ReportFixture(b2Fixture* fixture, const b2Vec2& point,
                                            const b2Vec2& normal, float fraction) {
                    UUID entityID = *(UUID*)&fixture->GetBody()->GetUserData();
                    Entity entity = m_Scene->getEntityById(entityID);
                    float distance = glm::distance(m_Point0, m_Point1) * fraction;
                    m_Results.emplace_back(entity, glm::vec2(point.x, point.y), glm::vec2(normal.x, normal.y),
                                           distance);
                    return 1.0f;
                }

            private:
                Ref<Scene> m_Scene;
                std::vector<Raycast2DResult>& m_Results;
                glm::vec2 m_Point0, m_Point1;
            };

            auto point0 = inRaycastData->Origin;
            auto point1 = inRaycastData->Origin + inRaycastData->Direction * inRaycastData->MaxDistance;
            auto& b2dWorld = scene->getPhysicsWorld();
            std::vector<Raycast2DResult> raycastResults;
            RayCastCallback callback(scene, raycastResults, point0, point1);
            b2dWorld.RayCast(&callback, {point0.x, point0.y}, {point1.x, point1.y});

            auto result = Coral::Array<ScriptRaycastHit2D>::New(raycastResults.size());

            for (size_t i = 0; i < raycastResults.size(); i++) {
                result[i] = {
                    .EntityID = raycastResults[i].HitEntity.getUUID(),
                    .Position = raycastResults[i].Point,
                    .Normal = raycastResults[i].Normal,
                    .Distance = raycastResults[i].Distance,
                };
            }
            return result;
        }

#pragma endregion

#pragma region Log

        void Log_LogMessage(LogLevel level, Coral::String inFormattedMessage) {
            std::string message = inFormattedMessage;
            switch (level) {
            case LogLevel::Trace:
                SHADO_TRACE(message);
                break;
            case LogLevel::Debug:
                SHADO_TRACE(message);
                break;
            case LogLevel::Info:
                SHADO_INFO(message);
                break;
            case LogLevel::Warn:
                SHADO_WARN(message);
                break;
            case LogLevel::Error:
                SHADO_ERROR(message);
                break;
            case LogLevel::Critical:
                SHADO_CRITICAL(message);
                break;
            }
            Coral::String::Free(inFormattedMessage);
        }

#pragma endregion

#pragma region Input

        Coral::Bool32 Input_IsKeyPressed(KeyCode keycode) { return Input::isKeyPressed(keycode); }

        Coral::Bool32 Input_IsMouseButtonPressed(int button) {
            bool isPressed = Input::isMouseButtonPressed(button);

            bool enableImGui = true;
            if (isPressed && enableImGui && GImGui->HoveredWindow != nullptr) {
                // Make sure we're in the viewport panel
                ImGuiWindow* viewportWindow = ImGui::FindWindowByName("Viewport");
                if (viewportWindow != nullptr)
                    isPressed = GImGui->HoveredWindow->ID == viewportWindow->ID;
            }

            return isPressed;
        }

        void Input_GetMousePosition(glm::vec2* outPosition) {
            auto [x, y] = Shado::Input::getMousePosition();
            *outPosition = {x, y};
        }

        //void Input_SetCursorMode(CursorMode mode) { Input::SetCursorMode(mode); }
        //CursorMode Input_GetCursorMode() { return Input::GetCursorMode(); }


#pragma endregion

#pragma region Project

        Coral::String Project_GetAssetDirectory() {
            if (Project::GetActive()) {
                return Coral::String::New(Project::GetAssetDirectory().lexically_normal().string());               
            }
            return Coral::String::New("");
        }

        Coral::String Project_GetProjectDirectory() {
            if (Project::GetActive()) {
                return Coral::String::New(Project::GetProjectDirectory().lexically_normal().string());               
            }
            return Coral::String::New("");            
        }

        Coral::String Project_GetRelativePath(Coral::String path) {
            SHADO_CORE_ASSERT(Project::GetActive(), "No active project!");
            return Coral::String::New(Project::GetActive()->GetRelativePath(std::string(path)).lexically_normal().string());
        }
#pragma endregion 
    }
}
