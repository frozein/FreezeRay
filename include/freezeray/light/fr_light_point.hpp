/* fr_light_point.hpp
 *
 * contains a definition for a point light source
 */

#ifndef FR_LIGHT_POINT_H
#define FR_LIGHT_POINT_H

#include "../fr_light.hpp"

//-------------------------------------------//

namespace fr
{

class LightPoint : public Light
{
public:
	LightPoint(const vec3& pos, const vec3& intensity);

	vec3 sample_li(const IntersectionInfo& hitInfo, const vec3& u, vec3& wiWorld, VisibilityTestInfo& vis, float& pdf) const override;
	float pdf_li(const IntersectionInfo& hitInfo, const vec3& w) const override;
	vec3 power() const override;

	vec3 sample_le(const vec3& u1, const vec3& u2, Ray& ray, vec3& normal, float& pdfPos, float& pdfDir) const override;
	void pdf_le(const Ray& ray, const vec3& normal, float& pdfPos, float& pdfDir) const override;

private:
	vec3 m_pos;
	vec3 m_intensity;
};

} //namespace fr

#endif //#ifndef FR_LIGHT_POINT_H