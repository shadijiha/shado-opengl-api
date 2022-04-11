#include "SceneHierarchyPanel.h"
#include "scene/Components.h"

#include <glm/gtc/type_ptr.hpp>

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
		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, tc.tag.c_str());
		if (ImGui::IsItemClicked()) {
			m_Selected = entity;
		}

		if (opened) {
			ImGui::TreePop();
		}
	}

	void SceneHierarchyPanel::drawComponents(Entity entity) {

		drawComponent<TagComponent>("Tag", entity, [this, &entity]() {
			auto& tc = entity.getComponent<TagComponent>();

			char buffer[512];
			memset(buffer, 0, sizeof(buffer));
			strcpy_s(buffer, sizeof(buffer), tc.tag.c_str());
			if (ImGui::InputText("Tag", buffer, sizeof(buffer))) {
				tc.tag = std::string(buffer);
			}
		});

		drawComponent<TransformComponent>("Transform", entity, [this, &entity]() {
			auto& transform = entity.getComponent<TransformComponent>();
			ImGui::DragFloat3("Position", glm::value_ptr(transform.transform[3]), 0.1f);
		});

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
				OrbitCamera* cam = (OrbitCamera*)camera.camera;

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
}
