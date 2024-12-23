/* light_spotlight.hpp
 *
 * contains a definition for a spotlight light source
 */

#ifndef RURT_LIGHT_SPOTLIGHT_H
#define RURT_LIGHT_SPOTLIGHT_H

#include "../light.hpp"

//-------------------------------------------//

namespace rurt
{

//spotlights point towards the -y direction by default
class LightSpotlight : public Light
{
public:
	LightSpotlight(const mat4& transform, const vec3& intensity, float width, float falloffStart);

	vec3 sample_li(const HitInfo& hitInfo, const vec2& u, vec3& wiWorld, VisibilityTestInfo& vis, float& pdf) const override;
	vec3 power() const;

private:
	mat4 m_toLocal;
	vec3 m_pos;

	vec3 m_intensity;
	float m_cosWidth;
	float m_cosFalloffStart;
};

} //namespace rurt

#endif //#ifndef RURT_LIGHT_SPOTLIGHT_H