/* brdf_specular.hpp
 *
 * contains a definition for a specular BRDF model
 */

#ifndef RURT_BRDF_SPECULAR_H
#define RURT_BRDF_SPECULAR_H

#include "../bxdf.hpp"
#include "../fresnel.hpp"

//-------------------------------------------//

namespace rurt
{

class BRDFSpecular : public BXDF
{
public:
	BRDFSpecular(std::shared_ptr<const Fresnel> fresnel);

	vec3 f(const vec3& wi, const vec3& wo) const override;
	vec3 sample_f(vec3& wi, const vec3& wo, const vec2& u, float& pdf) const override;
	float pdf(const vec3& wi, const vec3& wo) const override;

private:
	std::shared_ptr<const Fresnel> m_fresnel;
};

};

#endif //#ifndef RURT_BRDF_SPECULAR_H