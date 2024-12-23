#include "rurt/light/light_point.hpp"
#include "rurt/globals.hpp"

//-------------------------------------------//

namespace rurt
{

LightPoint::LightPoint(const vec3& pos, const vec3& intensity) :
	Light(true), m_pos(pos), m_intensity(intensity)
{

}

vec3 LightPoint::sample_li(const IntersectionInfo& hitInfo, const vec2& u, vec3& wiWorld, VisibilityTestInfo& vis, float& pdf) const
{
	vec3 toLight = m_pos - hitInfo.worldPos;

	wiWorld = toLight;
	pdf = 1.0f;
	vis.infinite = false;
	vis.endPos = m_pos;

	return m_intensity / dot(toLight, toLight);
}

vec3 LightPoint::power() const
{
	return 4.0f * RURT_PI * m_intensity;
}

}; //namespace rurt