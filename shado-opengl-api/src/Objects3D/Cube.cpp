#include "Cube.h"

namespace Shado {
	
	Cube::Cube() {

        float cube_vertices[] = {
            // front
            -1.0, -1.0,  1.0,
             1.0, -1.0,  1.0,
             1.0,  1.0,  1.0,
            -1.0,  1.0,  1.0,
            // back
            -1.0, -1.0, -1.0,
             1.0, -1.0, -1.0,
             1.0,  1.0, -1.0,
            -1.0,  1.0, -1.0
        };

		uint32_t cube_elements[] = {
			// front
			0, 1, 2,
			2, 3, 0,
			// right
			1, 5, 6,
			6, 2, 1,
			// back
			7, 6, 5,
			5, 4, 7,
			// left
			4, 0, 3,
			3, 7, 4,
			// bottom
			4, 5, 1,
			1, 0, 4,
			// top
			3, 2, 6,
			6, 7, 3
		};

		vertexBuffer = VertexBuffer::create(cube_vertices, sizeof(cube_vertices));
		vertexBuffer->setLayout({
			{ShaderDataType::Float3, "a_Position"}
		});
		
		indexBuffer = IndexBuffer::create(cube_elements, sizeof(cube_elements) / sizeof(uint32_t));

		vao = VertexArray::create();
		vao->addVertexBuffer(vertexBuffer);
		vao->setIndexBuffer(indexBuffer);		
	}
	
}
