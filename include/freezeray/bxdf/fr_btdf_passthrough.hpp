/* fr_btdf_specular.hpp
 *
 * contains a definition for a passthrough BTDF model, which just sets wi = -wo
 */

#ifndef FR_BTDF_PASSTHROUGH_H
#define FR_BTDF_PASSTHROUGH_H

#include "../fr_bxdf.hpp"

//-------------------------------------------//

namespace fr
{

class BTDFPassthrough : public BXDF
{
public:
	BTDFPassthrough();

	vec3 f(const vec3& wi, const vec3& wo) const override;
	vec3 sample_f(vec3& wi, const vec3& wo, const vec2& u, float& pdf) const override;
	float pdf(const vec3& wi, const vec3& wo) const override;
};

}; //namespace fr

#endif //#ifndef FR_BTDF_PASSTHROUGH_H