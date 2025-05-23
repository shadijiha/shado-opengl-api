#pragma once
#include "scene/Scene.h"
#include "util/TimeStep.h"
#include "scene/Entity.h"
#include "PropertiesPanel.h"

namespace Shado {

	class SceneHierarchyPanel
	{
	public:
		SceneHierarchyPanel(const std::string& title = "Scene Hierarchy", const std::string& propertiesPanelTitle = "Properties");
		SceneHierarchyPanel(const Ref<Scene>& scene, const std::string& title = "Scene Hierarchy", const std::string& propertiesPanelTitle = "Properties");

		void setSelected(Entity entity);
		void setContext(const Ref<Scene>& scene);

		void onImGuiRender();
		void onEvent(Event& e);

		Entity getSelected() const { return m_Selected; }
		void resetSelection();
		Ref<Scene> getContext() const { return m_Context; }

	private:
		void drawEntityNode(Entity e);

	public:
		inline static std::string SceneHeirarchyEntityDragDropId = "Scene_heirarchy_drag_entity";
	private:
		Ref<Scene> m_Context;
		Entity m_Selected;
		std::string m_Title;

		PropertiesPanel m_PropertiesPanel;
	};
}

