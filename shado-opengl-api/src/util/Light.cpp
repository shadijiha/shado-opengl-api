#include "Light.h"

namespace Shado {
	Light::Light()
		: m_LightColor(1, 1, 1)
	{
	}

	Light::Light(const glm::vec3& color)
		: m_LightColor(color)
	{
	}

	void Light::setColor(const glm::vec3& color) {
		m_LightColor = color;
	}

	// =====================================

	DiffuseLight::DiffuseLight()
		: Light(), m_LightPosition(0, 0, 0)
	{
	}

	DiffuseLight::DiffuseLight(const glm::vec3& position)
		: Light(), m_LightPosition(position)
	{
	}

	DiffuseLight::DiffuseLight(const glm::vec3& position, const glm::vec3& color)
		: Light(color), m_LightPosition(position)
	{
		m_LightColor = color;
	}

	void DiffuseLight::setPosition(const glm::vec3& position) {
		m_LightPosition = position;
	}	
}
