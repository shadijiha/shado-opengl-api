#include "Entity.h"
#include "box2d/b2_world.h"
#include "box2d/b2_fixture.h"
#include "box2d/b2_polygon_shape.h"
#include "renderer/Renderer2D.h"


namespace Shado {

	Entity::Entity()
		: m_Scene(nullptr), m_EntityHandle(entt::null)
	{
	}

	Entity::Entity(entt::entity handle, Scene* scene)
		: m_Scene(scene), m_EntityHandle(handle)
	{

	}

	Entity::~Entity() {
	}
}
