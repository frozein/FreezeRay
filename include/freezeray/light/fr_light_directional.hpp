/* fr_light_directional.hpp
 *
 * contains a definition for a directional light source
 */

#ifndef FR_LIGHT_DIRECTIONAL_H
#define FR_LIGHT_DIRECTIONAL_H

#include "../fr_light.hpp"

//-------------------------------------------//

namespace fr
{

class LightDirectional : public Light
{
public:
	LightDirectional(const vec3& dir, const vec3& intensity, float worldRadius = 1.0f);

	vec3 sample_li(const IntersectionInfo& hitInfo, const vec3& u, vec3& wiWorld, VisibilityTestInfo& vis, float& pdf) const override;
	float pdf_li(const IntersectionInfo& hitInfo, const vec3& w) const override;
	vec3 power() const override;

private:
	vec3 m_dir;
	vec3 m_intensity;
	float m_worldRadius;
};

} //namespace fr

#endif //#ifndef FR_LIGHT_DIRECTIONAL_H