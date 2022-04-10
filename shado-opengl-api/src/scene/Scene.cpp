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

		// Render stuff
		auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
		for(auto entity : group) {
			auto& [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);

			Renderer2D::DrawQuad(transform.transform, sprite.color);
		}
	}
}
