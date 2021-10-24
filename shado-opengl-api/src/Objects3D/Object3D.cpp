#include <fstream>
#include "Object3D.h"
#include "../Application.h"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"

namespace Shado {

	Object3D::Object3D(const std::string& filename) {

		using namespace std;

		vector<glm::vec4> vertices;
		vector<glm::vec3> normals;
		vector<uint32_t> elements;

		ifstream in(filename, ios::in);
		if (!in)
		{
			SHADO_CORE_ERROR("Cannot open {0}", filename);
		}

		string line;
		while (getline(in, line))
		{
			if (line.substr(0, 2) == "v ")
			{
				istringstream s(line.substr(2));
				glm::vec4 v; s >> v.x; s >> v.y; s >> v.z; v.w = 1.0f;
				vertices.push_back(v);
			} else if (line.substr(0, 2) == "f ")
			{
				istringstream s(line.substr(2));
				uint32_t a, b, c;
				s >> a; s >> b; s >> c;
				a--; b--; c--;
				elements.push_back(a); elements.push_back(b); elements.push_back(c);
			} else if (line[0] == '#')
			{
				/* ignoring this line */
			} else
			{
				/* ignoring this line */
			}
		}

		normals.resize(vertices.size(), glm::vec3(0.0, 0.0, 0.0));
		for (int i = 0; i < elements.size(); i += 3)
		{
			GLushort ia = elements[i];
			GLushort ib = elements[i + 1];
			GLushort ic = elements[i + 2];
			glm::vec3 normal = glm::normalize(glm::cross(
				glm::vec3(vertices[ib]) - glm::vec3(vertices[ia]),
				glm::vec3(vertices[ic]) - glm::vec3(vertices[ia])));
			normals[ia] = normals[ib] = normals[ic] = normal;
		}


		// Merge vertecies and Normals to 1 Vector to upload it to GPU
		vector<float> vecticesNormals;

		for (int i = 0; i < vertices.size(); i++) {

			const auto& vertex = vertices[i];
			const auto& normal = normals[i];

			vecticesNormals.push_back(vertex.x);
			vecticesNormals.push_back(vertex.y);
			vecticesNormals.push_back(vertex.z);
			vecticesNormals.push_back(vertex.w);

			vecticesNormals.push_back(normal.x);
			vecticesNormals.push_back(normal.y);
			vecticesNormals.push_back(normal.z);
		}

		vertexBuffer = VertexBuffer::create(&vecticesNormals[0], sizeof(float) * vecticesNormals.size());
		indexBuffer = IndexBuffer::create(&elements[0], elements.size());

		vertexBuffer->setLayout({
			{ShaderDataType::Float4, "a_Position"},
			{ShaderDataType::Float3, "a_Normal"  }
			});

		vao = VertexArray::create();
		vao->setIndexBuffer(indexBuffer);
		vao->addVertexBuffer(vertexBuffer);

		SHADO_CORE_INFO("Vertecies {0}", vertices.size());
	}
}
