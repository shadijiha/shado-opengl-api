#include "Layer.h"

namespace Shado {

	Layer::Layer(const std::string& name)
		: m_Name(name), m_Id(rand() % UINT64_MAX)
	{
	}

	////////////////// SCENE //////////////////
	Scene::Scene(const std::string& name)
		: name(name)
	{
	}

	Scene::~Scene() {
		for (Layer* layer : m_Layers) {
			delete layer;
		}
	}

	void Scene::pushLayer(Layer* layer) {
		m_Layers.push_back(layer);
	}

	const std::vector<Layer*>& Scene::getLayers() const {
		return m_Layers;
	}

}
