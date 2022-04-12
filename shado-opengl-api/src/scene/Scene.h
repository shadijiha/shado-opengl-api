#pragma once
#include "entt.hpp"
#include "util/Util.h"

namespace Shado {
	class Entity;

	class Scene {
	public:
		Scene();
		~Scene();

		Entity createEntity(const std::string& name = "");
		void destroyEntity(Entity entity);

		void onUpdate(TimeStep ts);
		void onDraw();

		void onViewportResize(uint32_t width, uint32_t height);

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
