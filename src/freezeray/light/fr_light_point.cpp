#include "freezeray/light/fr_light_point.hpp"
#include "freezeray/fr_globals.hpp"

//-------------------------------------------//

namespace fr
{

LightPoint::LightPoint(const vec3& pos, const vec3& intensity) :
	Light(true), m_pos(pos), m_intensity(intensity)
{

}

vec3 LightPoint::sample_li(const IntersectionInfo& hitInfo, const vec3& u, vec3& wiWorld, VisibilityTestInfo& vis, float& pdf) const
{
	vec3 toLight = m_pos - hitInfo.worldPos;

	wiWorld = normalize(toLight);
	pdf = 1.0f;
	vis.infinite = false;
	vis.endPos = m_pos;

	return m_intensity / dot(toLight, toLight);
}

vec3 LightPoint::power() const
{
	return 4.0f * FR_PI * m_intensity;
}

}; //namespace fr