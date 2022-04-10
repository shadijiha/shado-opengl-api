#pragma once
#include "entt.hpp"
#include "util/Util.h"

namespace Shado {
	class Entity;

	class Scene {
	public:
		Scene();
		~Scene();

		Entity createEntity(const std::string& name = "");

		void onUpdate(TimeStep ts);
		void onDraw();

	private:
		entt::registry m_Registry;

		friend class Entity;
	};

}
