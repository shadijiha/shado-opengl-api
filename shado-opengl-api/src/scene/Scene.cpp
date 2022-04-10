#include "Scene.h"
#include "Components.h"
#include "renderer/Renderer2D.h"
#include "Entity.h"
namespace Shado {

	Scene::Scene() {
	}

	Scene::~Scene() {
	}

	Entity Scene::createEntity(const std::string& name) {
		entt::entity id = m_Registry.create();
		Entity entity = { id, this };

		entity.addComponent<TransformComponent>();
		auto& tag = entity.addComponent<TagComponent>();
		tag.tag = name.empty() ? std::string("Entity ") + std::to_string((uint32_t)id) : name;

		return entity;
	}

	void Scene::onUpdate(TimeStep ts) {
	}

	void Scene::onDraw() {

		// Render 2D: Cameras
		Camera* primaryCamera = nullptr;
		glm::mat4* cameraTransform = nullptr;
		{
			// Loop through ortho cameras
			auto group = m_Registry.view<TransformComponent, OrthoCameraComponent>();
			for (auto entity : group) {
				auto& [transform, camera] = group.get<TransformComponent, OrthoCameraComponent>(entity);

				if (camera.primary) {
					primaryCamera = &camera.camera;
					cameraTransform = &transform.transform;
					break;
				}
			}

			// Loop through Orbit cameras
			auto orbitCams = m_Registry.view<TransformComponent, OrbitCameraComponent>();
			for (auto entity : orbitCams) {
				auto& [transform, camera] = orbitCams.get<TransformComponent, OrbitCameraComponent>(entity);

				if (camera.primary) {
					primaryCamera = &camera.camera;
					cameraTransform = &transform.transform;
					break;
				}
			}
		}

		
		if (primaryCamera) {
			Renderer2D::BeginScene(*primaryCamera, *cameraTransform);

			// Render stuff
			auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
			for (auto entity : group) {
				auto& [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);

				Renderer2D::DrawQuad(transform.transform, sprite.color);
			}

			Renderer2D::EndScene();
		}	
	}

	void Scene::onViewportResize(uint32_t width, uint32_t height) {
		m_ViewportWidth = width;
		m_ViewportHeight = height;

		// Resize cams
		// Loop through ortho cameras
		auto orthCams = m_Registry.view<OrthoCameraComponent>();
		for (auto entity : orthCams) {
			auto& camera = orthCams.get<OrthoCameraComponent>(entity);

			if (!camera.fixedAspectRatio) {
				camera.setViewportSize(width, height);
			}
		}

		// Loop through Orbit cameras
		auto orbitCams = m_Registry.view<OrbitCameraComponent>();
		for (auto entity : orbitCams) {
			auto& camera = orbitCams.get<OrbitCameraComponent>(entity);

			if (!camera.fixedAspectRatio) {
				camera.setViewportSize(width, height);
			}
		}
	}
}
