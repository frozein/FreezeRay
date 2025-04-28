#include "freezeray/fr_prng.hpp"

#include "freezeray/fr_globals.hpp"

//-------------------------------------------//

namespace fr
{

PRNG::PRNG(uint32_t seed) :
	m_generator(seed), m_distF(0.0f, 1.0f)
{

}

float PRNG::randf()
{
	return m_distF(m_generator);
}

vec2 PRNG::rand2f()
{
	return vec2(randf(), randf());
}

vec3 PRNG::rand3f()
{
	return vec3(randf(), randf(), randf());
}

vec3 PRNG::rand_sphere()
{
	float theta = 2.0f * FR_PI * randf();
	float phi = std::acosf(1.0f - 2.0f * randf());
	
	float sin_phi = std::sinf(phi);
	return vec3(
		std::cosf(theta) * sin_phi,
		std::sinf(theta) * sin_phi,
		std::cosf(phi)
	);
}

vec3 PRNG::rand_hemisphere(const vec3& normal)
{
	vec3 spherePoint = rand_sphere();
	if(dot(spherePoint, normal) < 0.0f)
		spherePoint = -1.0f * spherePoint;
		
	return spherePoint;
}

}; //namespace fr