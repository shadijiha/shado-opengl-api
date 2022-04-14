#pragma once
#include "entt.hpp"
#include "cameras/EditorCamera.h"
#include "ui/UUID.h"
#include "util/Util.h"

class b2World;

namespace Shado {
	class Entity;

	class Scene {
	public:
		Scene();
		Scene(Scene& other);
		~Scene();

		Entity createEntity(const std::string& name = "");
		Entity createEntityWithUUID(const std::string& name, Shado::UUID id);
		void destroyEntity(Entity entity);

		void onRuntimeStart();
		void onRuntimeStop();

		void onUpdateRuntime(TimeStep ts);
		void onDrawRuntime();

		void onUpdateEditor(TimeStep ts, EditorCamera& camera);
		void onDrawEditor(EditorCamera& camera);

		void onViewportResize(uint32_t width, uint32_t height);
		Entity getPrimaryCameraEntity();

	private:
		entt::registry m_Registry;
		uint32_t m_ViewportWidth = 0;
		uint32_t m_ViewportHeight = 0;
		std::string name = "Untitled";

		b2World* m_World = nullptr;

		friend class Entity;
		friend class SceneSerializer;
		friend class SceneHierarchyPanel;
	};

}
