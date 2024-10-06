#pragma once
#include "entt.hpp"
//#include "Physics2DCallback.h"
#include <filesystem>

#include "cameras/EditorCamera.h"
#include "script/ScriptEntityStorage.hpp"
#include "ui/UUID.h"
#include "util/TimeStep.h"

class b2World;

namespace Shado {
    class Entity;
    class Prefab;

    class SceneChangedEvent : public Event {
    public:
        SceneChangedEvent(std::filesystem::path path)
            : sceneToLoadPath(std::move(path)) {
        }

        EVENT_CLASS_TYPE(SceneChanged);
        EVENT_CLASS_CATEGORY(EventCategoryScene);
        const std::filesystem::path sceneToLoadPath;
    };

    class Scene : public RefCounted {
    public:
        Scene();
        Scene(Scene& other);
        ~Scene();

        Entity createEntity(const std::string& name = "");
        Entity createEntityWithUUID(const std::string& name, UUID id);
        Entity duplicateEntity(Entity entity, bool modifyTag = true);
        void destroyEntity(Entity entity);

        Entity instantiatePrefab(Ref<Prefab> prefab, bool modifyTag = true);
        void propagatePrefabChanges(Ref<Prefab> prefabChanged);

        void onRuntimeStart();
        void onRuntimeStop();

        void onUpdateRuntime(TimeStep ts);
        void onDrawRuntime();

        void onUpdateEditor(TimeStep ts, EditorCamera& camera);
        void onDrawEditor(EditorCamera& camera);

        void onViewportResize(uint32_t width, uint32_t height);

        Entity getPrimaryCameraEntity();
        Entity getEntityById(uint64_t id);
        Entity findEntityByName(std::string_view name);
        const entt::registry& getRegistry() { return m_Registry; }
        std::vector<Entity> getAllEntities();

        glm::vec2 getViewport() const {
            return {m_ViewportWidth, m_ViewportHeight};
        }

        void enablePhysics(bool cond) { m_PhysicsEnabled = cond; }
        void softResetPhysics();
        // Mainly used so if you use gizmos while playing the scene, it retains the position during the runtime
        b2World& getPhysicsWorld() const { return *m_World; }

        bool isRunning() const { return m_IsRunning; }
        ScriptStorage& GetScriptStorage();

        inline static Ref<Scene> ActiveScene = nullptr; // TODO: remove this
    private:
        Entity instantiatePrefabHelper(Ref<Prefab> prefab, Entity toDuplicate, bool modifyTag = true);

    private:
        entt::registry m_Registry;
        uint32_t m_ViewportWidth = 0;
        uint32_t m_ViewportHeight = 0;
        std::string name = "Untitled";

        std::vector<Entity> toDestroy;

        b2World* m_World = nullptr;
        bool m_PhysicsEnabled = true;

        bool m_IsRunning = false;
        ScriptStorage m_ScriptStorage;

        friend class Entity;
        friend class SceneSerializer;
        friend class SceneHierarchyPanel;
        friend class PropertiesPanel;
        friend class SceneInfoPanel;
    };

    // Utility functions
    void CopyRegistries(entt::registry& source, entt::registry& dest, std::function<Entity(const std::string&, UUID)>);
}
