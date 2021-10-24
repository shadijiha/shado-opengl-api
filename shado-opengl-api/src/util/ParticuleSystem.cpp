#include "ParticuleSystem.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/compatibility.hpp"
#include "Random.h"
#include "../Renderer2D.h"

namespace Shado {

	ParticuleSystem::ParticuleSystem() {
		m_ParticulePool.resize(1000);
	}

	void ParticuleSystem::emit(const ParticuleProps& props) {

		Particule& particule = m_ParticulePool[m_PoolIndex];
		particule.Active = true;
		particule.position = props.position;
		particule.rotation = { 0, 0, Random::Float() * 2.0f * glm::pi<float>() };

		// Velocity
		particule.velocity = props.velocity;
		particule.velocity.x += props.velocityVariation.x * (Random::Float() - 0.5f);
		particule.velocity.y += props.velocityVariation.y * (Random::Float() - 0.5f);
		particule.velocity.z += props.velocityVariation.z * (Random::Float() - 0.5f);

		// Color
		particule.ColorBegin = props.colorBegin;
		particule.ColorEnd = props.colorEnd;

		// Size
		particule.SizeBegin = props.sizeBegin + props.sizeVariation * (Random::Float() - 0.5f);
		particule.SizeEnd = props.sizeEnd;

		// Life
		particule.LifeTime = props.lifeTime;
		particule.LifeRemaining = props.lifeTime;

		m_PoolIndex = --m_PoolIndex % m_ParticulePool.size();

	}

	void ParticuleSystem::onUpdate(TimeStep ts) {

		for (auto& particule : m_ParticulePool) {

			if (!particule.Active)
				continue;

			if (particule.LifeRemaining <= 0.0f) {
				particule.Active = false;
				continue;
			}

			particule.LifeRemaining -= ts;
			particule.position += particule.velocity * (float)ts;
			particule.rotation.z += 0.01f * ts;

		}

	}

	void ParticuleSystem::onDraw() {

		for (auto& particle : m_ParticulePool) {

			float life = particle.LifeRemaining / particle.LifeTime;
			glm::vec4 color = glm::lerp(particle.ColorBegin, particle.ColorEnd, life);
			color.a = color.a * life;
			float size = glm::lerp(particle.SizeBegin, particle.SizeEnd, life);

			Renderer2D::DrawRotatedQuad(particle.position, { size, size }, particle.rotation, color);

		}
	}

}
