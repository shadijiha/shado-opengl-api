#pragma once
#include <glm/glm.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include "EditorEvents.h"
#include "Entity.h"
#include "cameras/Camera.h"
#include "cameras/OrbitCamera.h"
#include "cameras/OrthoCamera.h"
#include "renderer/Font.h"
#include "renderer/Shader.h"
#include "renderer/Texture2D.h"
#include "script/CSharpObject.h"

namespace Shado {
    struct Component {};

    struct IDComponent : Component {
        UUID id;
    };

    struct TagComponent : Component {
        std::string tag;

        TagComponent() = default;

        TagComponent(const TagComponent&) = default;

        TagComponent(const std::string& tag) : tag(tag) {}
    };

    struct TransformComponent : Component {
        glm::vec3 position = {0, 0, 0};
        glm::vec3 rotation = {0, 0, 0};
        glm::vec3 scale = {1.0f, 1.0f, 1.0f};
        UUID parentId = 0;

        TransformComponent() = default;

        TransformComponent(const TransformComponent&) = default;

        TransformComponent(const glm::vec3& position) : position(position) {}

        /**
         * Fetches the total transform of the entity. Meaning localtransform + parent transform recursively
         * @param scene The scene where the entity was created
         * @return Returns the transform matrix of the entity
         */
        glm::mat4 getTransform(Scene& scene) const {
            glm::mat4 _rotation = glm::toMat4(glm::quat(rotation));
            glm::mat4 localTransform = glm::translate(glm::mat4(1.0f), position) * _rotation * glm::scale(
                glm::mat4(1.0f), scale);

            Entity parent = getParent(scene);
            if (parent.isValid()) {
                return parent.getComponent<TransformComponent>().getTransform(scene) + localTransform;
            } else {
                return localTransform;
            }
        }

        /**
         * Fetches the total position of the entity. Meaning localPosition + parent position recursively
         * @param scene The scene where the entity was created 
         * @return Returns the position matrix of the entity
         */
        glm::vec3 getPosition(Scene& scene) const {
            Entity parent = getParent(scene);
            if (parent.isValid()) {
                return parent.getComponent<TransformComponent>().getPosition(scene) + position;
            } else {
                return position;
            }
        }

        glm::mat4 getLocalTransform() const {
            glm::mat4 _rotation = glm::toMat4(glm::quat(rotation));
            return glm::translate(glm::mat4(1.0f), position) * _rotation * glm::scale(glm::mat4(1.0f), scale);
        }

        void setParent(Entity target, Entity parent) {
            // Push undo event
            Application::dispatchEvent(
                EditorEntityChanged(EditorEntityChanged::ChangeType::ENTITY_PARENT_CHANGED, target));

            // Change entity parent
            if (parent)
                this->parentId = parent.getUUID();
            else {
                SHADO_CORE_WARN("setParent was called with invalid parent entity");
                this->parentId = 0;
            }
        }

        Entity getParent(Scene& sceneToLookup) const {
            return sceneToLookup.getEntityById(parentId);
        }
    };

    struct SpriteRendererComponent : Component {
        glm::vec4 color = {1, 1, 1, 1};
        AssetHandle texture = 0;
        float tilingFactor = 1.0f;
        AssetHandle shader = 0;


        SpriteRendererComponent() = default;

        SpriteRendererComponent(const SpriteRendererComponent&) = default;

        SpriteRendererComponent(const glm::vec4& color) : color(color) {}

        //~SpriteRendererComponent() { delete texture; }
    };

    /// IMPORTANT!!! Must have the same layout as SpriteRendererComponent because of drawTextureControl in PropertiesPanel.cpp
    struct CircleRendererComponent : Component {
        glm::vec4 color = {1, 1, 1, 1};
        AssetHandle texture = 0;
        float tilingFactor = 1.0f;
        AssetHandle shader = 0;
        float thickness = 1.0f;
        float fade = 0.005f;

        CircleRendererComponent() = default;

        //~CircleRendererComponent() { delete texture; }
    };

    struct LineRendererComponent : Component {
        glm::vec3 target = {0, 0, 0};
        glm::vec4 color = {1, 1, 1, 1};
    };

    struct CameraComponent : Component {
        enum class Type {
            Orthographic = 0, Orbit = 1
        };

        Ref<Camera> camera;
        bool primary = true;
        bool fixedAspectRatio = false;
        Type type = Type::Orthographic;

        CameraComponent(Type type, uint32_t width, uint32_t height): type(type) {
            cachedWidth = width;
            cachedHeight = height;

            init(width, height);
        }

        CameraComponent(const CameraComponent&) = default;

        CameraComponent()
            : CameraComponent(Type::Orthographic,
                              Application::get().getWindow().getWidth(),
                              Application::get().getWindow().getHeight()) {}

        ~CameraComponent() {}

        void setViewportSize(uint32_t width, uint32_t height) {
            if (type == Type::Orthographic) {
                auto* cam = (OrthoCamera*)camera.Raw();
                float aspectRatio = (float)width / (float)height;
                float left = -size * aspectRatio * 0.5f;
                float right = size * aspectRatio * 0.5f;
                float bottom = -size * 0.5f;
                float top = size * 0.5f;
                cam->setProjection(left, right, bottom, top);
            } else {
                auto* cam = (OrbitCamera*)camera.Raw();
                cam->setAspectRatio((float)width / (float)height);
            }

            cachedWidth = width;
            cachedHeight = height;
        }

        float getSize() const {
            if (type == Type::Orbit)
                SHADO_CORE_WARN("Attempting to get the size of an orbit camera?");
            return size;
        }

        void setSize(float size) {
            if (type == Type::Orthographic) {
                this->size = size;
                this->setViewportSize(cachedWidth, cachedHeight);
            }
        }

        void setType(Type type) {
            camera.Reset();
            this->type = type;
            init(cachedWidth, cachedHeight);
        }

    private:
        float size = 5.0f;
        uint32_t cachedWidth, cachedHeight;

    private:
        void init(uint32_t width, uint32_t height) {
            if (type == Type::Orthographic) {
                camera = CreateRef<OrthoCamera>(-16.0f, 16.0f, -9.0f, 9.0f);
                setViewportSize(width, height);
            } else
                camera = CreateRef<OrbitCamera>((float)width / (float)height);
        }
    };

    struct NativeScriptComponent : Component {
        ScriptableEntity* script;

        ScriptableEntity* (*instantiateScript)();

        void (*destroyScript)(NativeScriptComponent*);

        template <typename T>
        void bind() {
            instantiateScript = []() { return (ScriptableEntity*)snew(T) T(); };
            destroyScript = [](NativeScriptComponent* nsc) {
                sdelete(nsc->script);
                nsc->script = nullptr;
            };
        }
    };

    struct ScriptComponent : Component {
        UUID ScriptID = 0;
        CSharpObject Instance;
        std::vector<uint32_t> FieldIDs;

        // NOTE(Peter): Gets set to true when OnCreate has been called for this entity
        bool IsRuntimeInitialized = false;
    };

    // Physics
    struct RigidBody2DComponent : Component {
        enum class BodyType { STATIC = 0, KINEMATIC, DYNAMIC };

        BodyType type = BodyType::STATIC;
        bool fixedRotation = false;

        // b2 body
        void* runtimeBody = nullptr;

        RigidBody2DComponent() = default;

        RigidBody2DComponent(const RigidBody2DComponent&) = default;
    };

    struct BoxCollider2DComponent : Component {
        glm::vec2 offset = {0, 0};
        glm::vec2 size = {0.5f, 0.5f};

        // TODO : Should be moved to a physics material
        float density = 1.0f;
        float friction = 0.5f;
        float restitution = 0;
        float restitutionThreshold = 0.5f;

        // b2 Fixture
        void* runtimeFixture = nullptr;

        BoxCollider2DComponent() = default;

        BoxCollider2DComponent(const BoxCollider2DComponent&) = default;
    };

    struct CircleCollider2DComponent : Component {
        glm::vec2 offset = {0.0f, 0.0f};
        glm::vec2 radius = glm::vec2(0.5f); // The Y component is ignored, this is just here to be able
        // To type pun in ScriptManager setupInteralCalls

        // TODO: move into physics material in the future maybe
        float density = 1.0f;
        float friction = 0.5f;
        float restitution = 0.0f;
        float restitutionThreshold = 0.5f;

        // Storage for runtime
        void* runtimeFixture = nullptr;

        CircleCollider2DComponent() = default;

        CircleCollider2DComponent(const CircleCollider2DComponent&) = default;
    };

    struct PrefabInstanceComponent : Component {
        UUID prefabId = 0;

        // Each entity inside the prefab has a unique id (different from the prefab id)
        UUID prefabEntityUniqueId = 0;
        // <-- We need this so we can propogate changes the prefab changes to the prefab instances

        // Is valid?
        operator bool() const {
            return prefabId != 0 && prefabEntityUniqueId != 0;
        }
    };

    struct TextComponent : Component {
        Ref<Font> font = nullptr;
        std::string text = "";
        glm::vec4 color = {1, 1, 1, 1};
        float lineSpacing = 0.0f;
        float kerning = 0.0f;

        TextComponent() = default;

        TextComponent(const TextComponent&) = default;
    };

    template <typename... Components>
    struct ComponentGroup {};

    using AllComponents =
    ComponentGroup<TagComponent, TransformComponent, SpriteRendererComponent,
                   CircleRendererComponent, LineRendererComponent, CameraComponent, ScriptComponent,
                   NativeScriptComponent, RigidBody2DComponent, BoxCollider2DComponent,
                   CircleCollider2DComponent, PrefabInstanceComponent, TextComponent>;
}
