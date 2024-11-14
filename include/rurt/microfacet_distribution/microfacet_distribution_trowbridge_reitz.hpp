/* microfacet_distribution_trowbridge_reitz.hpp
 * 
 * contains a definition for a trowbridge-reitz microfacet distribution
 */

#ifndef RURT_MICROFACET_DISTRIBUTION_TROWBRIDGE_REITZ_H
#define RURT_MICROFACET_DISTRIBUTION_TROWBRIDGE_REITZ_H

#include "microfacet_distribution.hpp"

//-------------------------------------------//

namespace rurt
{

class MicrofacetDistributionTrowbridgeReitz : public MicrofacetDistribution
{
public:
	MicrofacetDistributionTrowbridgeReitz(float roughnessX, float roughnessY);

	float distribution(const vec3& w) const override;
	float invisible_masked_proportion(const vec3& w) const override;

private:
	float m_alphaX;
	float m_alphaY;
	
	static float roughness_to_alpha(float roughness);
};

} //namespace rurt

#endif //#ifndef RURT_MICROFACET_DISTRIBUTION_TROWBRIDGE_REITZ_H