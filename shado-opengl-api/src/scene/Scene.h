#pragma once
#include "entt.hpp"
//#include "Physics2DCallback.h"
#include "cameras/EditorCamera.h"
#include "ui/UUID.h"
#include "util/Util.h"

class b2World;

namespace Shado {
	class Entity;
	class Prefab;

	class Scene : public RefCounted  {
	public:
		Scene();
		Scene(Scene& other);
		~Scene();

		Entity createEntity(const std::string& name = "");
		Entity createEntityWithUUID(const std::string& name, Shado::UUID id);
		Entity duplicateEntity(Entity entity);
		void destroyEntity(Entity entity);
		Entity instantiatePrefab(Ref<Prefab> prefab);

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
		glm::vec2 getViewport() const {
			return { m_ViewportWidth , m_ViewportHeight };
		}

		void enablePhysics(bool cond) { m_PhysicsEnabled = cond; }
		void softResetPhysics();	// Mainly used so if you use gizmos while playing the scene, it retains the position during the runtime

		bool isRunning() const { return m_IsRunning; }
	public:
		inline static Ref<Scene> ActiveScene = nullptr; // TODO: remove this

	private:
		entt::registry m_Registry;
		uint32_t m_ViewportWidth = 0;
		uint32_t m_ViewportHeight = 0;
		std::string name = "Untitled";

		std::vector<Entity> toDestroy;

		b2World* m_World = nullptr;
		bool m_PhysicsEnabled = true;

		bool m_IsRunning = false;

		friend class Entity;
		friend class SceneSerializer;
		friend class SceneHierarchyPanel;
	};

}
