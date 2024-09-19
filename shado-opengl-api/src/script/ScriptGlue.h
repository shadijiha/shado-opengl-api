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
#pragma endregion

#pragma region Entity

        uint64_t Entity_GetParent(uint64_t entityID);
        void Entity_SetParent(uint64_t entityID, uint64_t parentID);

        Coral::Array<uint64_t> Entity_GetChildren(uint64_t entityID);

        void Entity_CreateComponent(uint64_t entityID, Coral::ReflectionType componentType);
        bool Entity_HasComponent(uint64_t entityID, Coral::ReflectionType componentType);
        bool Entity_RemoveComponent(uint64_t entityID, Coral::ReflectionType componentType);

#pragma endregion

#pragma region Scene
        bool Scene_IsEntityValid(uint64_t entityID);
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

#pragma endregion

#pragma region SpriteRendererComponent

        void SpriteRendererComponent_GetColor(uint64_t entityID, glm::vec4* outColor);
        void SpriteRendererComponent_SetColor(uint64_t entityID, glm::vec4* inColor);
        float SpriteRendererComponent_GetTilingFactor(uint64_t entityID);
        void SpriteRendererComponent_SetTilingFactor(uint64_t entityID, float tilingFactor);
        intptr_t SpriteRendererComponent_GetTexture(uint64_t entityID);
        void SpriteRendererComponent_SetTexture(uint64_t entityID, intptr_t inTexture);

#pragma endregion

#pragma region RigidBody2DComponent

        RigidBody2DComponent::BodyType RigidBody2DComponent_GetBodyType(uint64_t entityID);
        void RigidBody2DComponent_SetBodyType(uint64_t entityID, RigidBody2DComponent::BodyType inType);

#pragma endregion

#pragma region BoxCollider2DComponent


#pragma endregion

#pragma region TextComponent

        Coral::String TextComponent_GetText(uint64_t entityID);
        void TextComponent_SetText(uint64_t entityID, Coral::String text);
        void TextComponent_GetColor(uint64_t entityID, glm::vec4* outColor);
        void TextComponent_SetColor(uint64_t entityID, glm::vec4* inColor);

#pragma endregion

#pragma region Texture2D

        intptr_t Texture2D_Create(Coral::String inPath/*, OutParam<intptr_t> outHandle*/);
        //void Texture2D_GetSize(intptr_t inHandle, uint32_t* outWidth, uint32_t* outHeight);
        //void Texture2D_SetData(intptr_t inHandle, Coral::Array<glm::vec4> inData);
        //Coral::Array Texture2D_GetData(Param<AssetHandle> inHandle);

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
