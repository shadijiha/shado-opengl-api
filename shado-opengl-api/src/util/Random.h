#pragma once
#include <random>

namespace Shado {

	class Random
	{
	public:
		static void init();

		static float Float();
		static float Float(float min, float max);

		static double Double();
		static double Double(double min, double max);

		static int Int();
		static int Int(int min, int max);
		
	private:
		static std::mt19937 s_RandomEngine;
		static std::uniform_int_distribution<std::mt19937::result_type> s_Distribution;
		static bool hasInit;
	};
	
}
