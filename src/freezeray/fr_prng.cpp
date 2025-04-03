#include "freezeray/fr_prng.hpp"

#include "freezeray/fr_globals.hpp"

//-------------------------------------------//

namespace fr
{

PRNG::PRNG() :
	m_generator(), m_distI(0, UINT32_MAX), m_distF(0.0f, 1.0f)
{

}

uint32_t PRNG::randi()
{
	return m_distI(m_generator);
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
	vec3 randUnitSphere;
	while(true)
	{
		randUnitSphere = rand3f();

		float lenSqr = dot(randUnitSphere, randUnitSphere);
		if(lenSqr > FR_EPSILON && lenSqr <= 1.0f)
		{
			randUnitSphere = randUnitSphere / std::sqrtf(lenSqr);
			break;
		}
	}

	return randUnitSphere;
}

vec3 PRNG::rand_hemisphere(const vec3& normal)
{
	vec3 randUnitSphere;
	while(true)
	{
		randUnitSphere = rand3f();

		float lenSqr = dot(randUnitSphere, randUnitSphere);
		if(lenSqr > FR_EPSILON && lenSqr <= 1.0f)
		{
			randUnitSphere = randUnitSphere / std::sqrtf(lenSqr);
			break;
		}
	}

	if(dot(randUnitSphere, normal) < 0.0f)
		randUnitSphere = randUnitSphere * -1.0f;

	return randUnitSphere;
}

}; //namespace fr