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

	void Scene::destroyEntity(Entity entity) {
		m_Registry.destroy(entity);
	}

	void Scene::onUpdate(TimeStep ts) {
		// Update script
		{
			m_Registry.view<NativeScriptComponent>().each([this, ts](auto entity, NativeScriptComponent& nsc) {
				// TODO: mave to onScenePlay
				if (!nsc.script) {
					nsc.script = nsc.instantiateScript();
					nsc.script->m_EntityHandle = entity;
					nsc.script->m_Scene = this;
					nsc.script->onCreate();					
				}

				nsc.script->onUpdate(ts);
			});
		}
	}

	void Scene::onDraw() {

		// Render 2D: Cameras
		Camera* primaryCamera = nullptr;
		glm::mat4 cameraTransform;
		{
			// Loop through ortho cameras
			auto group = m_Registry.view<TransformComponent, CameraComponent>();
			for (auto entity : group) {
				auto [transform, camera] = group.get<TransformComponent, CameraComponent>(entity);

				if (camera.primary) {
					primaryCamera = camera.camera.get();
					cameraTransform = transform.getTransform();
					break;
				}
			}
		}

		
		if (primaryCamera) {
			Renderer2D::BeginScene(*primaryCamera, cameraTransform);

			// Render stuff
			auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
			for (auto entity : group) {
				auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);

				Renderer2D::DrawQuad(transform.getTransform(), sprite.color);
			}

			Renderer2D::EndScene();
		}	
	}

	void Scene::onViewportResize(uint32_t width, uint32_t height) {
		m_ViewportWidth = width;
		m_ViewportHeight = height;

		// Resize cams
		// Loop through ortho cameras
		auto orthCams = m_Registry.view<CameraComponent>();
		for (auto entity : orthCams) {
			auto& camera = orthCams.get<CameraComponent>(entity);

			if (!camera.fixedAspectRatio) {
				camera.setViewportSize(width, height);
			}
		}
	}
}
