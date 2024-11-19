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
	BRDFMicrofacet(const vec3& color, std::shared_ptr<const MicrofacetDistribution> distribution, std::shared_ptr<const Fresnel> fresnel);

	vec3 f(const HitInfo& info, const vec3& wi, const vec3& wo) const override;
	vec3 sample_f(const HitInfo& info, vec3& wi, const vec3& wo, float& pdf) const override;
	float pdf(const HitInfo& info, const vec3& wi, const vec3& wo) const override;

	bool is_delta() const override { return false; }

private:
	vec3 m_color;
	std::shared_ptr<const MicrofacetDistribution> m_distribution;
	std::shared_ptr<const Fresnel> m_fresnel;
};

}; //namespace rurt

#endif //#ifndef RURT_BRDF_MICROFACET_H