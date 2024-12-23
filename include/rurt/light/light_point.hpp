/* light_point.hpp
 *
 * contains a definition for a point light source
 */

#ifndef RURT_LIGHT_POINT_H
#define RURT_LIGHT_POINT_H

#include "../light.hpp"

//-------------------------------------------//

namespace rurt
{

class LightPoint : public Light
{
public:
	LightPoint(const vec3& pos, const vec3& intensity);

	vec3 sample_li(const HitInfo& hitInfo, const vec2& u, vec3& wiWorld, VisibilityTestInfo& vis, float& pdf) const override;
	vec3 power() const;

private:
	vec3 m_pos;
	vec3 m_intensity;
};

} //namespace rurt

#endif //#ifndef RURT_LIGHT_POINT_H