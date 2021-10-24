#pragma once
#include "glm/vec3.hpp"

namespace Shado {
	
	class Light {
	public:
		Light();
		Light(const glm::vec3& color);
		virtual ~Light() = default;

		const glm::vec3& getColor() const { return m_LightColor; }

		void setColor(const glm::vec3& color);		

	protected:
		glm::vec3 m_LightColor;
	};


	class DiffuseLight : public Light {
	public:
		DiffuseLight();
		DiffuseLight(const glm::vec3& position);
		DiffuseLight(const glm::vec3& position, const glm::vec3& color);

		const glm::vec3& getPosition() const { return m_LightPosition; }

		void setPosition(const glm::vec3& position);

	private:
		glm::vec3 m_LightPosition;
	};
}
