#pragma once
#include "Scene.h"

namespace Shado {
	class SceneSerializer {
	public:
		SceneSerializer(const Ref<Scene>& scene);
		SceneSerializer(const SceneSerializer&) = delete;

		void serialize(const std::string& filepath);
		void serializeRuntime(const std::string& filepath);

		bool deserialize(const std::string& filepath);
		bool deserializeRuntime(const std::string& filepath);
	private:
		Ref<Scene> m_Scene;
	};

}
