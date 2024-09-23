#pragma once
#include <string>
#include <Coral/Assembly.hpp>
#include <Coral/String.hpp>
#include <Coral/Array.hpp>

#include <glm/glm.hpp>

#include <concepts>

#include "scene/Components.h"

namespace Shado {
    class ScriptGlue {
    public:
        static void RegisterGlue(Coral::ManagedAssembly& coreAssembly);

    private:
        static void RegisterComponentTypes(Coral::ManagedAssembly& coreAssembly);
        static void RegisterInternalCalls(Coral::ManagedAssembly& coreAssembly);
    };

    namespace InternalCalls {
        // NOTE(Peter): "Hack" around C++ taking in types that aren't standard layout, e.g UUID isn't passable because
        //				it defines proper constructors, this method is slightly risky though, depending on the type
        template <std::default_initializable T>
        struct Param {
            std::byte Data[sizeof(T)];

            operator T() const {
                T result;
                std::memcpy(&result, Data, sizeof(T));
                return result;
            }
        };

        template <typename T>
        struct OutParam {
            std::byte* Ptr = nullptr;

            T* operator->() noexcept { return reinterpret_cast<T*>(Ptr); }
            const T* operator->() const noexcept { return reinterpret_cast<const T*>(Ptr); }

            T& operator*() { return *reinterpret_cast<T*>(Ptr); }
            const T& operator*() const { return *reinterpret_cast<const T*>(Ptr); }
        };

#pragma region Application
        void Application_Quit();
        float Application_GetTime();
        uint32_t Application_GetWidth();
        uint32_t Application_GetHeight();
        void Application_GetImGuiWindowSize(Coral::String name, glm::vec2* outSize);
        bool Application_IsImGuiWindowHovered(Coral::String windowName);
#pragma endregion

#pragma region Window
        void Window_GetPosition(glm::vec2* outPosition);
        void Window_GetSize(glm::vec2* outSize);
        void Window_SetSize(glm::vec2* inSize);
        int Window_GetMode();
        void Window_SetMode(int mode);
        Coral::String Window_GetTitle();
        void Window_SetTitle(Coral::String title);
        bool Window_GetVSync();
        void Window_SetVSync(bool enabled);
        void Window_SetOpacity(float opacity);
#pragma endregion

#pragma region Entity

        uint64_t Entity_GetParent(uint64_t entityID);
        void Entity_SetParent(uint64_t entityID, uint64_t parentID);

        Coral::Array<uint64_t> Entity_GetChildren(uint64_t entityID);

        void Entity_CreateComponent(uint64_t entityID, Coral::ReflectionType componentType);
        bool Entity_HasComponent(uint64_t entityID, Coral::ReflectionType componentType);
        bool Entity_RemoveComponent(uint64_t entityID, Coral::ReflectionType componentType);
        uint64_t Entity_FindEntityByName(Coral::String name);

#pragma endregion

#pragma region Prefab
        uint64_t Prefab_Instantiate(uint64_t prefabID, glm::vec3 position);
#pragma endregion

#pragma region Scene
        bool Scene_IsEntityValid(uint64_t entityID);
        Coral::String Scene_LoadScene(Coral::String scenePath);
        Coral::Array<uint64_t> Scene_GetAllEntities();
#pragma endregion

#pragma region TagComponent

        Coral::String TagComponent_GetTag(uint64_t entityID);
        void TagComponent_SetTag(uint64_t entityID, Coral::String inTag);

#pragma endregion

#pragma region TransformComponent

        struct Transform {
            glm::vec3 Translation = glm::vec3(0.0f);
            glm::vec3 Rotation = glm::vec3(0.0f);
            glm::vec3 Scale = glm::vec3(1.0f);
        };

        void TransformComponent_GetTransform(uint64_t entityID, Transform* outTransform);
        void TransformComponent_SetTransform(uint64_t entityID, Transform* inTransform);

#pragma endregion

#pragma region ScriptComponent

        Coral::ManagedObject ScriptComponent_GetInstance(uint64_t entityID);

#pragma endregion

#pragma region CameraComponent
        bool CameraComponent_GetPrimary(uint64_t entityID);
        void CameraComponent_SetPrimary(uint64_t entityID, bool inPrimary);
        CameraComponent::Type CameraComponent_GetType(uint64_t entityID);
        void CameraComponent_SetType(uint64_t entityID, CameraComponent::Type inType);
        void CameraComponent_SetViewportSize(uint64_t entityID, uint32_t inWidth, uint32_t inHeight);

#pragma endregion

#pragma region SpriteRendererComponent

        void SpriteRendererComponent_GetColor(uint64_t entityID, glm::vec4* outColor);
        void SpriteRendererComponent_SetColor(uint64_t entityID, glm::vec4* inColor);
        float SpriteRendererComponent_GetTilingFactor(uint64_t entityID);
        void SpriteRendererComponent_SetTilingFactor(uint64_t entityID, float tilingFactor);
        uint64_t SpriteRendererComponent_GetTexture(uint64_t entityID);
        void SpriteRendererComponent_SetTexture(uint64_t entityID, AssetHandle inTexture);
        uint64_t SpriteRendererComponent_GetShader(uint64_t entityID);
        void SpriteRendererComponent_SetShader(uint64_t entityID, AssetHandle inShader);

#pragma endregion

#pragma region CircleRendererComponent
        void CircleRendererComponent_GetColor(uint64_t entityID, glm::vec4* outColor);
        void CircleRendererComponent_SetColor(uint64_t entityID, glm::vec4* inColor);
        float CircleRendererComponent_GetThickness(uint64_t entityID);
        void CircleRendererComponent_SetThickness(uint64_t entityID, float inThickness);
        float CircleRendererComponent_GetFade(uint64_t entityID);
        void CircleRendererComponent_SetFade(uint64_t entityID, float inFade);
#pragma endregion

#pragma region LineRendererComponent
        void LineRendererComponent_GetTarget(uint64_t entityID, glm::vec3* outTarget);
        void LineRendererComponent_SetTarget(uint64_t entityID, glm::vec3* inTarget);
        void LineRendererComponent_GetColour(uint64_t entityID, glm::vec4* outColor);
        void LineRendererComponent_SetColour(uint64_t entityID, glm::vec4* inColor);
#pragma endregion

#pragma region RigidBody2DComponent

        void RigidBody2DComponent_GetLinearVelocity(uint64_t entityID, glm::vec2* outVelocity);
        RigidBody2DComponent::BodyType RigidBody2DComponent_GetBodyType(uint64_t entityID);
        void RigidBody2DComponent_SetBodyType(uint64_t entityID, RigidBody2DComponent::BodyType inType);
        void Rigidbody2DComponent_ApplyLinearImpulse(uint64_t entityID, glm::vec2 impulse, glm::vec2 worldPosition,
                                                     bool wake);
        void Rigidbody2DComponent_ApplyLinearImpulseToCenter(uint64_t entityID, glm::vec2 impulse, bool wake);

#pragma endregion

#pragma region BoxCollider2DComponent and CircleCollider2DComponent

        void BoxCollider2DComponent_GetOffset(uint64_t entityID, glm::vec2* outOffset);
        void BoxCollider2DComponent_SetOffset(uint64_t entityID, glm::vec2* inOffset);
        void BoxCollider2DComponent_GetSize(uint64_t entityID, glm::vec2* outSize);
        void BoxCollider2DComponent_SetSize(uint64_t entityID, glm::vec2* inSize);
        float BoxCollider2DComponent_GetDensity(uint64_t entityID);
        void BoxCollider2DComponent_SetDensity(uint64_t entityID, float inDensity);
        float BoxCollider2DComponent_GetFriction(uint64_t entityID);
        void BoxCollider2DComponent_SetFriction(uint64_t entityID, float inFriction);
        float BoxCollider2DComponent_GetRestitution(uint64_t entityID);
        void BoxCollider2DComponent_SetRestitution(uint64_t entityID, float inRestitution);
        float BoxCollider2DComponent_GetRestitutionThreshold(uint64_t entityID);
        void BoxCollider2DComponent_SetRestitutionThreshold(uint64_t entityID, float inRestitutionThreshold);

#pragma endregion

#pragma region TextComponent

        Coral::String TextComponent_GetText(uint64_t entityID);
        void TextComponent_SetText(uint64_t entityID, Coral::String text);
        void TextComponent_GetColor(uint64_t entityID, glm::vec4* outColor);
        void TextComponent_SetColor(uint64_t entityID, glm::vec4* inColor);
        float TextComponent_GetLineSpacing(uint64_t entityID);
        void TextComponent_SetLineSpacing(uint64_t entityID, float inLineSpacing);
        float TextComponent_GetKerning(uint64_t entityID);
        void TextComponent_SetKerning(uint64_t entityID, float inKerning);

#pragma endregion

#pragma region Texture2D

        uint64_t Texture2D_Create(Coral::String inPath);
        //void Texture2D_GetSize(intptr_t inHandle, uint32_t* outWidth, uint32_t* outHeight);
        //void Texture2D_SetData(intptr_t inHandle, Coral::Array<glm::vec4> inData);
        //Coral::Array Texture2D_GetData(Param<AssetHandle> inHandle);

#pragma endregion

#pragma region Shader

        uint64_t Shader_Create(Coral::String pathRelativeToProject);
        void Shader_SetInt(uint64_t inHandle, Coral::String inName, int inValue);
        void Shader_SetFloat(uint64_t inHandle, Coral::String inName, float inValue);
        void Shader_SetFloat2(uint64_t inHandle, Coral::String inName, glm::vec2 inValue);
        void Shader_SetFloat3(uint64_t inHandle, Coral::String inName, glm::vec3 inValue);
        void Shader_SetFloat4(uint64_t inHandle, Coral::String inName, glm::vec4 inValue);


#pragma endregion

#pragma region Log

        enum class LogLevel : int32_t {
            Trace = BIT(0),
            Debug = BIT(1),
            Info = BIT(2),
            Warn = BIT(3),
            Error = BIT(4),
            Critical = BIT(5)
        };


        void Log_LogMessage(LogLevel level, Coral::String inFormattedMessage);

#pragma endregion

#pragma region Input

        Coral::Bool32 Input_IsKeyPressed(KeyCode keycode);
        //Coral::Bool32 Input_IsKeyHeld(KeyCode keycode);
        //Coral::Bool32 Input_IsKeyDown(KeyCode keycode);
        //Coral::Bool32 Input_IsKeyReleased(KeyCode keycode);
        Coral::Bool32 Input_IsMouseButtonPressed(int button);
        //Coral::Bool32 Input_IsMouseButtonHeld(int button);
        //Coral::Bool32 Input_IsMouseButtonDown(int button);
        //Coral::Bool32 Input_IsMouseButtonReleased(int button);
        void Input_GetMousePosition(glm::vec2* outPosition);
        //void Input_SetCursorMode(CursorMode mode);
        //CursorMode Input_GetCursorMode();


#pragma endregion
    }
}
