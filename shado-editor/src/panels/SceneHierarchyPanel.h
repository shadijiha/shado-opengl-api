#pragma once
#include "scene/Scene.h"
#include "util/Util.h"
#include "scene/Entity.h"
#include "imgui.h"

namespace Shado {

	class SceneHierarchyPanel
	{
	public:
		SceneHierarchyPanel() = default;
		SceneHierarchyPanel(const Ref<Scene>& scene);

		void setSelected(Entity entity);
		void setContext(const Ref<Scene>& scene);

		void onImGuiRender();
		void onEvent(Event& e);

		Entity getSelected() const { return m_Selected; }
		void resetSelection();

	private:
		void drawEntityNode(Entity e);
		void drawComponents(Entity e);

	public:
		inline static std::string SceneHeirarchyEntityDragDropId = "Scene_heirarchy_drag_entity";
	private:
		Ref<Scene> m_Context;
		Entity m_Selected;
	};
}

