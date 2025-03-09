/* fr_brtdf_lambertian.hpp
 *
 * contains a definition for a lambertian BTDF model
 */

#ifndef FR_BTDF_LAMBERTIAN
#define FR_BTDF_LAMBERTIAN

#include "../fr_bxdf.hpp"

//-------------------------------------------//

namespace fr
{

class BTDFLambertian : public BXDF
{
public:
	BTDFLambertian();

	vec3 f(const vec3& wi, const vec3& wo) const override;
	vec3 sample_f(vec3& wi, const vec3& wo, const vec2& u, float& pdf) const override;
	float pdf(const vec3& wi, const vec3& wo) const override;
};

}; //namespace fr

#endif //#ifndef FR_BTDF_LAMBERTIAN