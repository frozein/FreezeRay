/* fr_prng.hpp
 *
 * contains the definition of the PRNG class, which
 * represents a pseudo-random number generator for use
 * across the program
 */

#ifndef FR_PRNG_H
#define FR_PRNG_H

#include <random>

#include "quickmath.hpp"
using namespace qm;

//-------------------------------------------//

namespace fr
{

class PRNG
{
public:
	PRNG();

	uint32_t randi();
	float randf();
	vec2 rand2f();
	vec3 rand3f();
	vec3 rand_sphere();
	vec3 rand_hemisphere(const vec3& normal);

private:
	std::mt19937 m_generator;
	std::uniform_int_distribution<uint32_t> m_distI;
	std::uniform_real_distribution<float> m_distF;
};

}

#endif //#ifndef FR_PRNG_H