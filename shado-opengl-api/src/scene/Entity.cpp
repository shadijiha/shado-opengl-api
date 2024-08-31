#include "Entity.h"
#include "box2d/b2_world.h"
#include "box2d/b2_fixture.h"
#include "box2d/b2_polygon_shape.h"
#include "renderer/Renderer2D.h"
#include "scene/Components.h"


namespace Shado {

	Entity::Entity()
		: m_Scene(nullptr), m_EntityHandle(entt::null), m_Registry(nullptr)
	{
	}

	Entity::Entity(entt::entity handle, Scene* scene)
		: m_Scene(scene), m_EntityHandle(handle), m_Registry(&scene->m_Registry)
	{

	}

	Entity::Entity(entt::entity handle, entt::registry* registry)
		: m_Scene(nullptr), m_EntityHandle(handle), m_Registry(registry)
	{
	}

	Entity::~Entity() {
	}
	UUID Entity::getUUID() const
	{
		return getComponent<IDComponent>().id;
	}

	std::vector<Entity> Entity::getChildren() const {
		std::vector<Entity> children;

		for (auto e : getRegistry().view<TransformComponent>()) {
			Entity currentIt;
			if (m_Scene != nullptr)
				currentIt = { e, m_Scene };
			else 
				currentIt = { e, m_Registry };

			const TransformComponent& tc = currentIt.getComponent<TransformComponent>();
			if (tc.parentId == this->getUUID()) {
				children.push_back(currentIt);
			}
		}

		return children;
	}

	bool Entity::isChild(Scene& sceneToLookup) const {
		return getComponent<TransformComponent>().getParent(sceneToLookup).isValid();
	}

	Entity Entity::findChildByTag(const std::string& tag) {
		// Find the entity with the tag
		for (auto child : getChildren()) {
			if (child.getComponent<TagComponent>().tag == tag) {
				return child;
			}
		}

		return  {};
	}
}
