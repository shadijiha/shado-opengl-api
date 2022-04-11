#pragma once
#include "scene/Scene.h"
#include "util/Util.h"
#include "scene/Entity.h"

namespace Shado {

	class SceneHierarchyPanel
	{
	public:
		SceneHierarchyPanel() = default;
		SceneHierarchyPanel(const Ref<Scene>& scene);

		void setContext(const Ref<Scene>& scene);

		void onImGuiRender();

	private:
		void drawEntityNode(Entity e);
	private:
		Ref<Scene> m_Context;
		Entity m_Selected;
	};
}
