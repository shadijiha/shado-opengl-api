#pragma once
#include "glm/vec3.hpp"
#include "Objects3D/Object3D.h"
#include "util/Light.h"

namespace Shado {

	inline std::string OBJECT3D_DEFAULT_SHADER_PATH = FILE_PATH + "assets\\Renderer3D.glsl";

	class Renderer3D {
	public:

		static void Init();
		static void Clear();
		static void BeginScene(const Camera& camera);
		static void EndScene();

		static void DrawModel(const Ref<Object3D>& mesh, const glm::vec3& position = { 0, 0, 0 },
			const glm::vec3& scale = { 1, 1, 1 }, const glm::vec4& modelColor = { 1, 1, 1, 1 }, const DiffuseLight& light = DiffuseLight(), bool fill = true);

		static void DrawRotatedModel(const Ref<Object3D>& mesh, const glm::vec3& position = { 0, 0, 0 },
			const glm::vec3& scale = { 1, 1, 1 }, const glm::vec3& rotation = { 0, 0, 0 }, const glm::vec4& modelColor = { 1, 1, 1, 1 }, const DiffuseLight& light = DiffuseLight(), bool fill = true);


	private:

		static void DrawTransformedModel(const Ref<Object3D>& mesh, const glm::mat4& transform, const glm::vec4& modelColor, const DiffuseLight& light, bool fill);
	};

}
