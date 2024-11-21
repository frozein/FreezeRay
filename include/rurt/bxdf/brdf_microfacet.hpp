/* brdf_microfacet.hpp
 *
 * contains a definition for a microfacet BRDF model
 */

#ifndef RURT_BRDF_MICROFACET_H
#define RURT_BRDF_MICROFACET_H

#include "../bxdf.hpp"
#include "../microfacet_distribution.hpp"
#include "../fresnel.hpp"

//-------------------------------------------//

namespace rurt
{

class BRDFMicrofacet : public BXDF
{
public:
	BRDFMicrofacet(std::shared_ptr<const MicrofacetDistribution> distribution, std::shared_ptr<const Fresnel> fresnel);

	vec3 f(const vec3& wi, const vec3& wo) const override;
	vec3 sample_f(vec3& wi, const vec3& wo, float& pdf) const override;
	float pdf(const vec3& wi, const vec3& wo) const override;

private:
	std::shared_ptr<const MicrofacetDistribution> m_distribution;
	std::shared_ptr<const Fresnel> m_fresnel;
};

}; //namespace rurt

#endif //#ifndef RURT_BRDF_MICROFACET_H