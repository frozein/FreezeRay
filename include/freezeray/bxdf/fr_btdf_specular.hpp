/* fr_btdf_specular.hpp
 *
 * contains a definition for a specular BTDF model
 */

#ifndef FR_BTDF_SPECULAR_H
#define FR_BTDF_SPECULAR_H

#include "../fr_bxdf.hpp"
#include "../fr_fresnel.hpp"

//-------------------------------------------//

namespace fr
{

class BTDFSpecular : public BXDF
{
public:
	BTDFSpecular(float etaI, float etaT, std::shared_ptr<const Fresnel> fresnel);

	vec3 f(const vec3& wi, const vec3& wo) const override;
	vec3 sample_f(vec3& wi, const vec3& wo, const vec2& u, float& pdf) const override;
	float pdf(const vec3& wi, const vec3& wo) const override;

private:
	float m_etaI;
	float m_etaT;
	std::shared_ptr<const Fresnel> m_fresnel;
};

};

#endif //#ifndef FR_BTDF_SPECULAR_H