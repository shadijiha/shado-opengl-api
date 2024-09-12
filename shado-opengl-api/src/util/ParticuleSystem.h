#pragma once
#include <vector>

#include "glm/vec3.hpp"
#include "glm/vec4.hpp"
#include "TimeStep.h"

namespace Shado {

	struct ParticuleProps {
		glm::vec3 position;
		glm::vec3 velocity, velocityVariation;
		glm::vec4 colorBegin;
		glm::vec4 colorEnd;
		float sizeBegin;
		float sizeEnd;
		float sizeVariation;
		float lifeTime = 1.0f;
	};
	
	class ParticuleSystem {
	public:
		ParticuleSystem();

		void emit(const ParticuleProps& props);

		void onUpdate(TimeStep ts);
		void onDraw();

		uint32_t poolSize() const { return m_ParticulePool.size(); }
	private:

		struct Particule {
			glm::vec3 position;
			glm::vec3 velocity;
			glm::vec4 ColorBegin, ColorEnd;
			glm::vec3 rotation = { 0.0f, 0.0, 0.0 };
			float SizeBegin, SizeEnd;

			float LifeTime = 1.0f;
			float LifeRemaining = 0.0f;

			bool Active = false;
		};

		std::vector<Particule> m_ParticulePool;
		uint32_t m_PoolIndex = 0;	
	};
	
}
