/* fr_microfacet_distribution_beckmann.hpp
 * 
 * contains a definition for a beckmann-spizzichino microfacet distribution
 */

#ifndef FR_MICROFACET_DISTRIBUTION_BECKMANN_H
#define FR_MICROFACET_DISTRIBUTION_BECKMANN_H

#include "../fr_microfacet_distribution.hpp"

//-------------------------------------------//

namespace fr
{

class MicrofacetDistributionBeckmann : public MicrofacetDistribution
{
public:
	MicrofacetDistributionBeckmann(float roughnessX, float roughnessY);

	float distribution(const vec3& w) const override;
	virtual vec3 sample_distribution(const vec3& w, const vec2& u) const override;
	float invisible_masked_proportion(const vec3& w) const override;

private:
	float m_alphaX;
	float m_alphaY;

	static float roughness_to_alpha(float roughness);
};

} //namespace fr

#endif //#ifndef FR_MICROFACET_DISTRIBUTION_BECKMANN_H