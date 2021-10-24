#pragma once
#include "../VertexArray.h"
#include "../cameras/Camera.h"

namespace Shado {

	class Object3D {
	public:
		Object3D(const std::string& filename);
		virtual ~Object3D() = default;


		Ref<VertexArray> getVertexArray() const { return  vao; }

	protected:
		Object3D() = default;

	protected:
		Ref<VertexArray> vao;
		Ref<IndexBuffer> indexBuffer;
		Ref<VertexBuffer> vertexBuffer;
	};

}
