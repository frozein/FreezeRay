/* brdf_disney_diffuse.hpp
 *
 * contains a definition for a lambertian diffuse BRDF model
 */

#ifndef RURT_BRDF_LAMBERTIAN_DIFFUSE
#define RURT_BRDF_LAMBERTIAN_DIFFUSE

#include "brdf.hpp"

//-------------------------------------------//

namespace rurt
{

class BRDFLambertianDiffuse : public BRDF
{
public:
	BRDFLambertianDiffuse(vec3 color);

	vec3 f(const HitInfo& info, const vec3& i, const vec3& o, float& pdf) const override;
	float pdf(const HitInfo& info, const vec3& i, const vec3& o) const override;

private:
	vec3 m_color;
};

}; //namespace rurt

#endif //#ifndef RURT_BRDF_LAMBERTIAN_DIFFUSE