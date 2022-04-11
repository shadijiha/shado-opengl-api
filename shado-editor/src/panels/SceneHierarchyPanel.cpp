#include "SceneHierarchyPanel.h"
#include "scene/Components.h"

#include <glm/gtc/type_ptr.hpp>

#include "imgui_internal.h"

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

		if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered()) {
			m_Selected = {};
		}

		// Context menu to create 
		if (ImGui::BeginPopupContextWindow(0, 1, false)) {
			if (ImGui::MenuItem("Create empty Entity"))
				m_Context->createEntity("Empty Entity");

			ImGui::EndPopup();
		}

		ImGui::End();

		ImGui::Begin("Properties");
		if (m_Selected) {
			drawComponents(m_Selected);

			// To Add components context menu
			if (ImGui::Button("+"))
				ImGui::OpenPopup("AddComponent");

			if (ImGui::BeginPopup("AddComponent")) {
				if (ImGui::MenuItem("Sprite renderer")) {
					m_Selected.addComponent<SpriteRendererComponent>();
					ImGui::CloseCurrentPopup();
				}

				if (ImGui::MenuItem("Camera")) {
					m_Selected.addComponent<CameraComponent>(CameraComponent::Type::Orthographic, m_Context->m_ViewportWidth, m_Context->m_ViewportHeight);
					ImGui::CloseCurrentPopup();
				}

				if (ImGui::MenuItem("Native script")) {
					m_Selected.addComponent<NativeScriptComponent>();
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}
		}
		ImGui::End();
	}

	void SceneHierarchyPanel::drawEntityNode(Entity entity) {

		TagComponent& tc = entity.getComponent<TagComponent>();
		
		auto flags = ImGuiTreeNodeFlags_OpenOnArrow | (m_Selected == entity ? ImGuiTreeNodeFlags_Selected : 0);
		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, tc.tag.c_str());
		if (ImGui::IsItemClicked()) {
			m_Selected = entity;
		}

		// Context menu item
		bool deleteEntity = false;
		if (ImGui::BeginPopupContextItem()) {
			if (ImGui::MenuItem("Delete Entity"))
				deleteEntity = true;

			ImGui::EndPopup();
		}

		if (opened) {
			ImGui::TreePop();
		}

		// Defer the entity deletion to avoid bugs
		if (deleteEntity) {
			m_Context->destroyEntity(entity);
			if (m_Selected == entity)
				m_Selected = {};
		}
	}

	// Helpers
	static void drawVec3Control(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f);

	void SceneHierarchyPanel::drawComponents(Entity entity) {

		drawComponent<TagComponent>("Tag", entity, [this, &entity]() {
			auto& tc = entity.getComponent<TagComponent>();

			char buffer[512];
			memset(buffer, 0, sizeof(buffer));
			strcpy_s(buffer, sizeof(buffer), tc.tag.c_str());
			if (ImGui::InputText("Tag", buffer, sizeof(buffer))) {
				tc.tag = std::string(buffer);
			}
		}, false);

		drawComponent<TransformComponent>("Transform", entity, [this, &entity]() {
			auto& transform = entity.getComponent<TransformComponent>();

			drawVec3Control("Position", transform.position);

			auto rotation = glm::degrees(transform.rotation);
			drawVec3Control("Rotation", rotation);
			transform.rotation = glm::radians(rotation);

			drawVec3Control("Scale", transform.scale, 1.0);

			///ImGui::DragFloat3("Position", glm::value_ptr(transform.position), 0.1f);
			//ImGui::DragFloat3("Rotation", glm::value_ptr(transform.rotation), 0.1f);
			//ImGui::DragFloat3("Scale", glm::value_ptr(transform.scale), 0.1f);
		}, false);

		drawComponent<SpriteRendererComponent>("Sprite", entity, [this, &entity]() {
			auto& sprit = entity.getComponent<SpriteRendererComponent>();
			ImGui::ColorEdit4("Colour", glm::value_ptr(sprit.color));
		});

		drawComponent<CameraComponent>("Camera", entity, [this, &entity]() {
			auto& camera = entity.getComponent<CameraComponent>();

			// Select menu to change camera type
			const char* projectionTypesString[] = { "Orthographic", "Orbit (Perspective)" };
			const char* currentProjectionType = projectionTypesString[(int)camera.type];
			if (ImGui::BeginCombo("Type", currentProjectionType)) {

				for (int i = 0; i < 2; i++) {
					bool isSelected = currentProjectionType == projectionTypesString[i];

					if (ImGui::Selectable(projectionTypesString[i], isSelected)) {
						currentProjectionType = projectionTypesString[i];

						// Change camera type
						camera.setType((CameraComponent::Type)i);
					}

					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}

				ImGui::EndCombo();
			}

			if (camera.type == CameraComponent::Type::Orthographic) {
				float size = camera.getSize();
				if (ImGui::DragFloat("Size", &size)) {
					camera.setSize(size);
				}
			}

			if (camera.type == CameraComponent::Type::Orbit) {
				OrbitCamera* cam = (OrbitCamera*)camera.camera.get();

				float fov = cam->getFOV();
				if (ImGui::DragFloat("FOV", &fov)) {
					cam->setFOV(fov);
				}
			}

			float nearClip = camera.camera->getNearClip(), farClip = camera.camera->getFarClip();
			if (ImGui::DragFloat("Near clip", &nearClip, 0.5f, camera.type == CameraComponent::Type::Orbit ? 1.0f : -30.0, 1000)) {
				camera.camera->setNearClip(nearClip);
			}

			if (ImGui::DragFloat("Far clip", &farClip)) {
				camera.camera->setFarClip(farClip);
			}

			ImGui::Checkbox("Primary", &camera.primary);
			ImGui::Checkbox("Fixed aspect ratio", &camera.fixedAspectRatio);

		});
	}

	static void drawVec3Control(const std::string& label, glm::vec3& values, float resetValue, float columnWidth) {
		ImGui::PushID(label.c_str());

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::Text(label.c_str());
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0, 0 });

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

		ImGui::PushStyleColor(ImGuiCol_Button, { 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.9f, 0.2f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0.8f, 0.1f, 0.15f, 1.0f });
		if (ImGui::Button("X", buttonSize))
			values.x = resetValue;
		ImGui::PopStyleColor(3);
		

		ImGui::SameLine();
		ImGui::DragFloat("##X", &values.x, 0.1f, 0, 0, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();


		ImGui::PushStyleColor(ImGuiCol_Button, { 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.3f, 0.8f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0.2f, 0.7f, 0.2f, 1.0f });
		if (ImGui::Button("Y", buttonSize))
			values.y = resetValue;
		ImGui::PopStyleColor(3);


		ImGui::SameLine();
		ImGui::DragFloat("##Y", &values.y, 0.1f, 0, 0, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();


		ImGui::PushStyleColor(ImGuiCol_Button, { 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.2f, 0.35f, 0.9f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0.1f, 0.25f, 0.8f, 1.0f });
		if (ImGui::Button("Z", buttonSize))
			values.z = resetValue;
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Z", &values.z, 0.1f, 0, 0, "%.2f");
		ImGui::PopItemWidth();

		ImGui::PopStyleVar();

		ImGui::Columns(1);

		ImGui::PopID();
	}
}
