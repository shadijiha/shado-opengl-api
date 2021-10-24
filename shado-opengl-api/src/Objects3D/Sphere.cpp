#include "Sphere.h"
#include <glm/gtc/constants.hpp>

namespace Shado {

	template<typename tVal>
	static tVal map_value(std::pair<tVal, tVal> a, std::pair<tVal, tVal> b, tVal inVal)
	{
		tVal inValNorm = inVal - a.first;
		tVal aUpperNorm = a.second - a.first;
		tVal normPosition = inValNorm / aUpperNorm;

		tVal bUpperNorm = b.second - b.first;
		tVal bValNorm = normPosition * bUpperNorm;
		tVal outVal = b.first + bValNorm;

		return outVal;
	}

	static const double PI = 3.14159265359;
	
	Sphere::Sphere(float radius, int resolution) {

        std::vector<float> vertices;
        std::vector<unsigned int> indices;
        const float dLambda = 2 * glm::pi<float>() / resolution;
        const float dPhi = glm::pi<float>() / resolution;
        unsigned int lastVertex = 0;


        for (int i = 0; i < resolution; ++i) {
            for (int j = 0; j < resolution; ++j) {

                float lambda1 = j * dLambda;
                float phi1 = i * dPhi;
                float lambda2 = j + 1 == resolution ? 2 * glm::pi<float>()
                    : (j + 1) * dLambda;
                float phi2 = i + 1 == resolution ? glm::pi<float>()
                    : (i + 1) * dPhi;

                // vertex 1
                vertices.emplace_back(cosf(lambda1) * sinf(phi1) * radius);
                vertices.emplace_back(cosf(phi1) * radius);
                vertices.emplace_back(sinf(lambda1) * sinf(phi1) * radius);

                // vertex 2
                vertices.emplace_back(cosf(lambda1) * sinf(phi2) * radius);
                vertices.emplace_back(cosf(phi2) * radius);
                vertices.emplace_back(sinf(lambda1) * sinf(phi2) * radius);

                // vertex 3
                vertices.emplace_back(cosf(lambda2) * sinf(phi1) * radius);
                vertices.emplace_back(cosf(phi1) * radius);
                vertices.emplace_back(sinf(lambda2) * sinf(phi1) * radius);

                // vertex 4
                vertices.emplace_back(cosf(lambda2) * sinf(phi2) * radius);
                vertices.emplace_back(cosf(phi2) * radius);
                vertices.emplace_back(sinf(lambda2) * sinf(phi2) * radius);

                indices.emplace_back(lastVertex);
                indices.emplace_back(lastVertex + 1);
                indices.emplace_back(lastVertex + 2);

                indices.emplace_back(lastVertex + 1);
                indices.emplace_back(lastVertex + 3);
                indices.emplace_back(lastVertex + 2);

                lastVertex += 4;
            }
        }

		vertexBuffer = VertexBuffer::create(&vertices[0], sizeof(float) * vertices.size());
		indexBuffer = IndexBuffer::create(&indices[0], indices.size());

		vertexBuffer->setLayout({
			{ShaderDataType::Float3, "a_Position"},
		});

		vao = VertexArray::create();
		vao->addVertexBuffer(vertexBuffer);
		vao->setIndexBuffer(indexBuffer);			
	}
}
