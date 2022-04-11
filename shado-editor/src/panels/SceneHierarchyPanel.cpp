#include "SceneHierarchyPanel.h"
#include "scene/Components.h"
#include "imgui.h"


namespace Shado {

	SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& scene) {
		setContext(scene);
	}

	void SceneHierarchyPanel::setContext(const Ref<Scene>& scene) {
		m_Context = scene;
	}

	void SceneHierarchyPanel::onImGuiRender() {
		ImGui::Begin("Scene Hierachy");

		m_Context->m_Registry.each([this](auto entityID) {
			Entity entity = { entityID, m_Context.get() };
			drawEntityNode(entity);
		});

		ImGui::End();
	}

	void SceneHierarchyPanel::drawEntityNode(Entity entity) {

		TagComponent& tc = entity.getComponent<TagComponent>();
		
		auto flags = ImGuiTreeNodeFlags_OpenOnArrow | (m_Selected == entity ? ImGuiTreeNodeFlags_Selected : 0);
		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, tc.tag.c_str());
		if (ImGui::IsItemClicked()) {
			m_Selected = entity;
		}

		if (opened) {
			ImGui::TreePop();
		}
	}
}
