#include "freezeray/light/fr_light_spotlight.hpp"
#include "freezeray/fr_globals.hpp"

//-------------------------------------------//

namespace fr
{

LightSpotlight::LightSpotlight(const mat4& transform, const vec3& intensity, float width, float falloffStart) :
	Light(true), m_toLocal(inverse(transform)), m_pos((transform * vec4(0.0f, 0.0f, 0.0f, 1.0f)).xyz()), 
	m_intensity(intensity), m_cosWidth(std::cosf(width)), m_cosFalloffStart(std::cosf(falloffStart))
{

}

vec3 LightSpotlight::sample_li(const IntersectionInfo& hitInfo, const vec3& u, vec3& wiWorld, VisibilityTestInfo& vis, float& pdf) const
{
	vec3 toLight = m_pos - hitInfo.worldPos;

	vec3 wl = (m_toLocal * vec4(-1.0f * toLight, 1.0f)).xyz();
	float cosTheta = cos_theta(wl);
	
	float falloff;
	if(cosTheta < m_cosWidth)
		falloff = 0.0f;
	else if(cosTheta > m_cosFalloffStart)
		falloff = 1.0f;
	else
	{
       float delta = (cosTheta - m_cosWidth) / (m_cosFalloffStart - m_cosWidth);
       falloff = delta * delta * delta * delta;
	}

	wiWorld = normalize(toLight);
	pdf = 1.0f;
	vis.infinite = false;
	vis.endPos = m_pos;

	return m_intensity * falloff / dot(toLight, toLight);
}

vec3 LightSpotlight::power() const
{
	return m_intensity * FR_2_PI * (1.0f - 0.5f * (m_cosFalloffStart + m_cosWidth));
}

}; //namespace fr