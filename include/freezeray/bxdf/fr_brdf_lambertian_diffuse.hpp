/* fr_brdf_lambertian_diffuse.hpp
 *
 * contains a definition for a lambertian diffuse BRDF model
 */

#ifndef FR_BRDF_LAMBERTIAN_DIFFUSE
#define FR_BRDF_LAMBERTIAN_DIFFUSE

#include "../fr_bxdf.hpp"

//-------------------------------------------//

namespace fr
{

class BRDFLambertianDiffuse : public BXDF
{
public:
	BRDFLambertianDiffuse();

	vec3 f(const vec3& wi, const vec3& wo) const override;
	vec3 sample_f(vec3& wi, const vec3& wo, const vec2& u, float& pdf) const override;
	float pdf(const vec3& wi, const vec3& wo) const override;
};

}; //namespace fr

#endif //#ifndef FR_BRDF_LAMBERTIAN_DIFFUSE