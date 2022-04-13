#pragma once
#include "entt.hpp"
#include "cameras/EditorCamera.h"
#include "util/Util.h"

namespace Shado {
	class Entity;

	class Scene {
	public:
		Scene();
		~Scene();

		Entity createEntity(const std::string& name = "");
		void destroyEntity(Entity entity);

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

		friend class Entity;
		friend class SceneSerializer;
		friend class SceneHierarchyPanel;
	};

}
