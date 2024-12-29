#include "rurt/light/light_directional.hpp"
#include "rurt/globals.hpp"

//-------------------------------------------//

namespace rurt
{

LightDirectional::LightDirectional(const vec3& dir, const vec3& intensity, float worldRadius) :
	Light(true), m_dir(normalize(dir)), m_intensity(intensity), m_worldRadius(worldRadius)
{

}

vec3 LightDirectional::sample_li(const IntersectionInfo& hitInfo, const vec3& u, vec3& wiWorld, VisibilityTestInfo& vis, float& pdf) const
{
	wiWorld = m_dir;
	pdf = 1.0f;
	vis.infinite = true;
	vis.endPos = vec3(0.0f);

	return m_intensity;
}

vec3 LightDirectional::power() const
{
	return RURT_PI * m_intensity * m_worldRadius * m_worldRadius;
}

}; //namespace rurt