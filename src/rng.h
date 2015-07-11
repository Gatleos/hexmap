#ifndef RANDOM_H
#define RANDOM_H

#include <random>

namespace rng {
	// Default random generator
	extern std::mt19937 r;
	// [0,5]
	extern int d6(std::mt19937& urng);
	// [0,360)
	extern int degrees(std::mt19937& urng);
	// [0.0,TAU)
	extern double radians(std::mt19937& urng);
	// [0,1]
	extern bool boolean(std::mt19937& urng);
	// Seed the default random generator (with system time)
	extern void init();
	// [lo,hi]
	extern int getInt(int lo, int hi, std::mt19937& urng);
	// [0,hi]
	extern int getInt(int hi, std::mt19937& urng);
	// Returns true one in x times
	extern bool oneInX(double x, std::mt19937& urng);
	// Returns true x in y times
	extern bool xInY(double x, double y, std::mt19937& urng);
	// Random distributions are mutable for some reason, so to write-protect
	// them they are wrapped in functions here
}

#endif
