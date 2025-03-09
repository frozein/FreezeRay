/* fr_brdf_lambertian.hpp
 *
 * contains a definition for a lambertian BRDF model
 */

#ifndef FR_BRDF_LAMBERTIAN
#define FR_BRDF_LAMBERTIAN

#include "../fr_bxdf.hpp"

//-------------------------------------------//

namespace fr
{

class BRDFLambertian : public BXDF
{
public:
	BRDFLambertian();

	vec3 f(const vec3& wi, const vec3& wo) const override;
	vec3 sample_f(vec3& wi, const vec3& wo, const vec2& u, float& pdf) const override;
	float pdf(const vec3& wi, const vec3& wo) const override;
};

}; //namespace fr

#endif //#ifndef FR_BRDF_LAMBERTIAN