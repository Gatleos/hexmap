#include <chrono>
#include "rng.h"

std::uniform_int_distribution<int> d6Gen(0, 5);
std::uniform_int_distribution<int> degreesGen(0, 359);
std::uniform_real_distribution<double> radiansGen(0.0, 6.28318530718);
std::bernoulli_distribution booleanGen(0.5);

namespace rng
{
	std::mt19937 r;

	void init() {
		r.seed((unsigned)std::chrono::system_clock::now().time_since_epoch().count());
	}
	
	int d6(std::mt19937& urng) {
		return d6Gen(urng);
	}
	
	int degrees(std::mt19937& urng) {
		return degreesGen(urng);
	}
	
	double radians(std::mt19937& urng) {
		return radiansGen(urng);
	}
	
	bool boolean(std::mt19937& urng) {
		return booleanGen(urng);
	}

	int getInt(int lo, int hi, std::mt19937& urng) {
		return std::uniform_int_distribution<int>(lo, hi)(urng);
	}

	int getInt(int hi, std::mt19937& urng) {
		return std::uniform_int_distribution<int>(0, hi)(urng);
	}
	bool oneInX(double x, std::mt19937& urng) {
		return std::bernoulli_distribution(1.0 / x)(urng);
	}
	bool xInY(double x, double y, std::mt19937& urng) {
		return std::bernoulli_distribution(x / y)(urng);
	}
}
