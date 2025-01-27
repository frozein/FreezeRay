/* fr_brdf_microfacet.hpp
 *
 * contains a definition for a microfacet BRDF model
 */

#ifndef FR_BRDF_MICROFACET_H
#define FR_BRDF_MICROFACET_H

#include "../fr_bxdf.hpp"
#include "../fr_microfacet_distribution.hpp"
#include "../fr_fresnel.hpp"

//-------------------------------------------//

namespace fr
{

class BRDFMicrofacet : public BXDF
{
public:
	BRDFMicrofacet(std::shared_ptr<const MicrofacetDistribution> distribution, std::shared_ptr<const Fresnel> fresnel);

	vec3 f(const vec3& wi, const vec3& wo) const override;
	vec3 sample_f(vec3& wi, const vec3& wo, const vec2& u, float& pdf) const override;
	float pdf(const vec3& wi, const vec3& wo) const override;

private:
	std::shared_ptr<const MicrofacetDistribution> m_distribution;
	std::shared_ptr<const Fresnel> m_fresnel;
};

}; //namespace fr

#endif //#ifndef FR_BRDF_MICROFACET_H