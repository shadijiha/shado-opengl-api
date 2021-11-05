#include "Layer.h"

#include "Debug.h"
#include <algorithm>

namespace Shado {

	/*Layer::Layer(const std::string& name)
		: m_Name(name), m_Id(rand() % UINT64_MAX)
	{
	}*/

	////////////////// SCENE //////////////////
	Scene::Scene(const std::string& name)
		: name(name),  world({0.0f, -3.0f})
	{
	}

	Scene::~Scene() {
		/*for (Layer* layer : m_Layers) {
			delete layer;
		}*/
	}

	void Scene::updatePhysics(TimeStep dt) {
		world.Step(dt, 6, 2);
	}

	/*void Scene::pushLayer(Layer* layer) {
		layer->m_Scene = this;
		m_Layers.push_back(layer);
	}*/

	Entity* Scene::addEntityToWorld(Entity* entity) {
		entities.push_back(entity);
		return entity;
	}

	Entity* Scene::addEntityToWorld(const EntityDefinition& def) {
		auto* temp = new Entity(def, world);
		entities.push_back(temp);
		return temp;
	}

	void Scene::setWorldGravity(const glm::vec2& gravity) {
		world.SetGravity({ gravity.x, gravity.y });
	}

	void Scene::destroyEntity(Entity* entity) {
		if (entity != nullptr) {
			entity->destroy();

			int i = 0;
			for (Entity* e : entities) {
				if (entity == e) {
					entities.erase(entities.begin() + i);
					delete entity;
					break;
				}
				i++;
			}			
		}
	}

	Entity& Scene::getEntity(const std::string& name) {
		auto it = std::find_if(entities.begin(), entities.end(), [&name](Entity* e) {return e->getName() == name; });

		if (it != entities.end())
		{
			// found element. it is an iterator to the first matching element.
			// if you really need the index, you can also get it:
			auto index = std::distance(entities.begin(), it);
			return *entities[index];
		}

		SHADO_CORE_ASSERT(false, "Invalid entity name");
	}

	Entity& Scene::getEntity(uint64_t id) {
		auto it = std::find_if(entities.begin(), entities.end(), [&id](Entity* e) {return e->getId() == id; });

		if (it != entities.end())
		{
			auto index = std::distance(entities.begin(), it);
			return *entities[index];
		}

		SHADO_CORE_ASSERT(false, "Invalid entity ID");
	}

	/*const std::vector<Layer*>& Scene::getLayers() const {
		return m_Layers;
	}*/

}
