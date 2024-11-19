/* microfacet_distribution_beckmann.hpp
 * 
 * contains a definition for a beckmann-spizzichino microfacet distribution
 */

#ifndef RURT_MICROFACET_DISTRIBUTION_BECKMANN_H
#define RURT_MICROFACET_DISTRIBUTION_BECKMANN_H

#include "../microfacet_distribution.hpp"

//-------------------------------------------//

namespace rurt
{

class MicrofacetDistributionBeckmann : public MicrofacetDistribution
{
public:
	MicrofacetDistributionBeckmann(float roughnessX, float roughnessY);

	float distribution(const vec3& w) const override;
	float invisible_masked_proportion(const vec3& w) const override;

private:
	float m_alphaX;
	float m_alphaY;

	static float roughness_to_alpha(float roughness);
};

} //namespace rurt

#endif //#ifndef RURT_MICROFACET_DISTRIBUTION_BECKMANN_H