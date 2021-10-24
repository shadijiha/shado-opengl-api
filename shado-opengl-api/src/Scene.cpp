#include "Scene.h"

namespace Shado {

	Scene::Scene(const std::string& name, int zIndex)
		: m_Name(name), m_Id(rand() % UINT64_MAX), zIndex(zIndex)
	{
	}

	Scene::Scene(const std::string& name)
		: Scene(name, 0)
	{
	}

}
