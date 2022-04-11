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
		void drawComponent(const char* label, Entity entity, std::function<void()> ui, bool allowDeletion = true);
	private:
		Ref<Scene> m_Context;
		Entity m_Selected;


	};


	template<typename T>
	inline void SceneHierarchyPanel::drawComponent(const char* label, Entity entity, std::function<void()> ui, bool allowDeletion) {
		const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap;

		if (entity.hasComponent<T>()) {
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 4, 4 });

			bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, label);
			ImGui::SameLine(ImGui::GetWindowWidth() - 25.0f);

			// Component settings menu
			if (ImGui::Button("...", {20, 20})) {
				ImGui::OpenPopup("ComponentSettings");
			}
			ImGui::PopStyleVar();

			bool removeComponent = false;
			if (ImGui::BeginPopup("ComponentSettings")	) {

				if (allowDeletion && ImGui::MenuItem("Remove component"))
					removeComponent = true;

				ImGui::EndPopup();
			}

			if (open) {


				ui();

				ImGui::TreePop();
			}

			if (removeComponent) {
				entity.removeComponent<T>();
			}
		}
	}

}

