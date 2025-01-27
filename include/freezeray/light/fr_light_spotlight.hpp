/* fr_light_spotlight.hpp
 *
 * contains a definition for a spotlight light source
 */

#ifndef FR_LIGHT_SPOTLIGHT_H
#define FR_LIGHT_SPOTLIGHT_H

#include "../fr_light.hpp"

//-------------------------------------------//

namespace fr
{

//spotlights point towards the -y direction by default
class LightSpotlight : public Light
{
public:
	LightSpotlight(const mat4& transform, const vec3& intensity, float width, float falloffStart);

	vec3 sample_li(const IntersectionInfo& hitInfo, const vec3& u, vec3& wiWorld, VisibilityTestInfo& vis, float& pdf) const override;
	vec3 power() const;

private:
	mat4 m_toLocal;
	vec3 m_pos;

	vec3 m_intensity;
	float m_cosWidth;
	float m_cosFalloffStart;
};

} //namespace fr

#endif //#ifndef FR_LIGHT_SPOTLIGHT_H