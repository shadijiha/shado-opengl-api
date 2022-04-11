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

		void setContext(const Ref<Scene>& scene);

		void onImGuiRender();

	private:
		void drawEntityNode(Entity e);
		void drawComponents(Entity e);

		template<typename T>
		void drawComponent(const char* label, Entity entity, std::function<void()> ui);
	private:
		Ref<Scene> m_Context;
		Entity m_Selected;


	};


	template<typename T>
	inline void SceneHierarchyPanel::drawComponent(const char* label, Entity entity, std::function<void()> ui) {
		if (entity.hasComponent<T>()) {
			if (ImGui::TreeNodeEx((void*)typeid(T).hash_code(),
				ImGuiTreeNodeFlags_DefaultOpen
				, label)) {
				ui();
				ImGui::TreePop();
			}
		}
	}

}

