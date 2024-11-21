/* brdf_lambertian_diffuse.hpp
 *
 * contains a definition for a lambertian diffuse BRDF model
 */

#ifndef RURT_BRDF_LAMBERTIAN_DIFFUSE
#define RURT_BRDF_LAMBERTIAN_DIFFUSE

#include "../bxdf.hpp"

//-------------------------------------------//

namespace rurt
{

class BRDFLambertianDiffuse : public BXDF
{
public:
	BRDFLambertianDiffuse();

	vec3 f(const vec3& wi, const vec3& wo) const override;
	vec3 sample_f(vec3& wi, const vec3& wo, float& pdf) const override;
	float pdf(const vec3& wi, const vec3& wo) const override;
};

}; //namespace rurt

#endif //#ifndef RURT_BRDF_LAMBERTIAN_DIFFUSE