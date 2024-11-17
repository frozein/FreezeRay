/* brdf_microfacet.hpp
 *
 * contains a definition for a microfacet BRDF model
 */

#ifndef RURT_BRDF_MICROFACET_H
#define RURT_BRDF_MICROFACET_H

#include "brdf.hpp"
#include "../microfacet_distribution/microfacet_distribution.hpp"

//-------------------------------------------//

namespace rurt
{

class BRDFMicrofacet : public BRDF
{
public:
	BRDFMicrofacet(const vec3& color, std::shared_ptr<const MicrofacetDistribution> distribution);

	vec3 f(const HitInfo& info, const vec3& wi, const vec3& wo) const override;
	vec3 sample_f(const HitInfo& info, vec3& wi, const vec3& wo, float& pdf) const override;
	float pdf(const HitInfo& info, const vec3& wi, const vec3& wo) const override;

	bool is_delta() const override { return false; }

private:
	vec3 m_color;
	std::shared_ptr<const MicrofacetDistribution> m_distribution;
};

}; //namespace rurt

#endif //#ifndef RURT_BRDF_MICROFACET_H