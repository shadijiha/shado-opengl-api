#pragma once
#include "Scene.h"

namespace Shado {
	class Prefab;

	class SceneSerializer {
	public:
		SceneSerializer(const Ref<Scene>& scene);
		SceneSerializer(const SceneSerializer&) = delete;

		void serialize(const std::string& filepath);
		void serializeRuntime(const std::string& filepath);
		UUID serializePrefab(Entity entity);

		bool deserialize(const std::string& filepath, std::string& error);
		bool deserialize(const std::string& filepath);
		bool deserializeRuntime(const std::string& filepath);
		Ref<Prefab> deserializePrefab(const std::string& filepath);
	private:
		Ref<Scene> m_Scene;
	};

}
