/* light_directional.hpp
 *
 * contains a definition for a directional light source
 */

#ifndef RURT_LIGHT_DIRECTIONAL_H
#define RURT_LIGHT_DIRECTIONAL_H

#include "../light.hpp"

//-------------------------------------------//

namespace rurt
{

class LightDirectional : public Light
{
public:
	LightDirectional(const vec3& dir, const vec3& intensity, float worldRadius = 1.0f);

	vec3 sample_li(const IntersectionInfo& hitInfo, const vec2& u, vec3& wiWorld, VisibilityTestInfo& vis, float& pdf) const override;
	vec3 power() const;

private:
	vec3 m_dir;
	vec3 m_intensity;
	float m_worldRadius;
};

} //namespace rurt

#endif //#ifndef RURT_LIGHT_DIRECTIONAL_H