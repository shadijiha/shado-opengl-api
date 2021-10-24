#include "Random.h"

namespace Shado {

	std::mt19937 Random::s_RandomEngine;
	std::uniform_int_distribution<std::mt19937::result_type> Random::s_Distribution;
	bool Random::hasInit = false;

	void Random::init() {
		if (hasInit)
			return;

		s_RandomEngine.seed(std::random_device()());

		hasInit = true;
	}

	float Random::Float() {
		return (float)s_Distribution(s_RandomEngine) / (float)std::numeric_limits<uint32_t>::max();
	}

	float Random::Float(float min, float max) {
		return Float() * (max - min) + min;
	}

	double Random::Double() {
		return (double)s_Distribution(s_RandomEngine) / (double)std::numeric_limits<uint32_t>::max();
	}

	double Random::Double(double min, double max) {
		return Double() * (max - min) + min;
	}

	int Random::Int() {
		return int(s_Distribution(s_RandomEngine) / std::numeric_limits<uint32_t>::max());
	}

	int Random::Int(int min, int max) {
		return int(Double() * double(max - min) + (double)min);
	}

}
