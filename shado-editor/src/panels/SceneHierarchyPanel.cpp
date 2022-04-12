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
		}
		ImGui::End();
	}

	void SceneHierarchyPanel::drawEntityNode(Entity entity) {

		TagComponent& tc = entity.getComponent<TagComponent>();
		
		auto flags = ImGuiTreeNodeFlags_OpenOnArrow | (m_Selected == entity ? ImGuiTreeNodeFlags_Selected : 0);
		flags |= ImGuiTreeNodeFlags_SpanAvailWidth;

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
	template<typename T>
	static void drawComponent(const char* label, Entity entity, std::function<void(T&)> ui, bool allowDeletion = true);
	static void drawVec3Control(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f);
	static void addComponentContextMenu(Entity m_Selected, uint32_t vpWidth, uint32_t vpHeight);

	void SceneHierarchyPanel::drawComponents(Entity entity) {

		drawComponent<TagComponent>("Tag", entity, [](TagComponent& tc) {
			char buffer[512];
			memset(buffer, 0, sizeof(buffer));
			strcpy_s(buffer, sizeof(buffer), tc.tag.c_str());
			if (ImGui::InputText("##Tag", buffer, sizeof(buffer))) {
				tc.tag = std::string(buffer);
			}
		}, false);

		// Add component context menu
		addComponentContextMenu(m_Selected, m_Context->m_ViewportWidth, m_Context->m_ViewportHeight);

		drawComponent<TransformComponent>("Transform", entity, [](TransformComponent& transform) {
			drawVec3Control("Position", transform.position);

			auto rotation = glm::degrees(transform.rotation);
			drawVec3Control("Rotation", rotation);
			transform.rotation = glm::radians(rotation);

			drawVec3Control("Scale", transform.scale, 1.0);
		}, false);

		drawComponent<SpriteRendererComponent>("Sprite", entity, [](SpriteRendererComponent& sprite) {
			ImGui::ColorEdit4("Colour", glm::value_ptr(sprite.color));
		});

		drawComponent<CameraComponent>("Camera", entity, [](CameraComponent& camera) {
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
	
	template<typename T>
	static void drawComponent(const char* label, Entity entity, std::function<void(T&)> ui, bool allowDeletion) {
		const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap
				| ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding
				| ImGuiTreeNodeFlags_AllowItemOverlap;

		if (entity.hasComponent<T>()) {
			auto& component = entity.getComponent<T>();
			ImVec2 contentRegionAvail = ImGui::GetContentRegionAvail();

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 4, 4 });
			float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			ImGui::Separator();

			bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, label);

			ImGui::PopStyleVar();

			// Component settings menu
			if (allowDeletion) {
				ImGui::SameLine(contentRegionAvail.x - lineHeight * 0.5f);
				if (ImGui::Button("...", { lineHeight, lineHeight })) {
					ImGui::OpenPopup("ComponentSettings");
				}
			}
			

			bool removeComponent = false;
			if (allowDeletion && ImGui::BeginPopup("ComponentSettings")) {

				if (ImGui::MenuItem("Remove component"))
					removeComponent = true;

				ImGui::EndPopup();
			}

			if (open) {
				ui(component);
				ImGui::TreePop();
			}

			if (removeComponent) {
				entity.removeComponent<T>();
			}
		}
	}

	// Draw a vector with colourful x, y and z
	static void drawVec3Control(const std::string& label, glm::vec3& values, float resetValue, float columnWidth) {
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[ImguiLayer::FontIndex::BOLD];

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
		ImGui::PushFont(boldFont);
		if (ImGui::Button("X", buttonSize))
			values.x = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);
		

		ImGui::SameLine();
		ImGui::DragFloat("##X", &values.x, 0.1f, 0, 0, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();


		ImGui::PushStyleColor(ImGuiCol_Button, { 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.3f, 0.8f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Y", buttonSize))
			values.y = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);


		ImGui::SameLine();
		ImGui::DragFloat("##Y", &values.y, 0.1f, 0, 0, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();


		ImGui::PushStyleColor(ImGuiCol_Button, { 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.2f, 0.35f, 0.9f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Z", buttonSize))
			values.z = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Z", &values.z, 0.1f, 0, 0, "%.2f");
		ImGui::PopItemWidth();

		ImGui::PopStyleVar();

		ImGui::Columns(1);

		ImGui::PopID();
	}

	static void addComponentContextMenu(Entity m_Selected, uint32_t vpWidth, uint32_t vpHeight) {
		ImGui::SameLine();
		ImGui::PushItemWidth(-1);
		// To Add components context menu
		if (ImGui::Button("Add Component"))
			ImGui::OpenPopup("AddComponent");

		if (ImGui::BeginPopup("AddComponent")) {
			if (ImGui::MenuItem("Sprite renderer")) {
				m_Selected.addComponent<SpriteRendererComponent>();
				ImGui::CloseCurrentPopup();
			}

			if (ImGui::MenuItem("Camera")) {
				m_Selected.addComponent<CameraComponent>(CameraComponent::Type::Orthographic, vpWidth, vpHeight);
				ImGui::CloseCurrentPopup();
			}

			if (ImGui::MenuItem("Native script")) {
				m_Selected.addComponent<NativeScriptComponent>();
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
		ImGui::PopItemWidth();
	}
}
