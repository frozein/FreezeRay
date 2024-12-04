/* btdf_microfacet.hpp
 *
 * contains a definition for a microfacet BTDF model
 */

#ifndef RURT_BTDF_MICROFACET_H
#define RURT_BTDF_MICROFACET_H

#include "../bxdf.hpp"
#include "../microfacet_distribution.hpp"
#include "../fresnel.hpp"

//-------------------------------------------//

namespace rurt
{

class BTDFMicrofacet : public BXDF
{
public:
	BTDFMicrofacet(float etaI, float etaT, std::shared_ptr<const MicrofacetDistribution> distribution, std::shared_ptr<const Fresnel> fresnel);

	vec3 f(const vec3& wi, const vec3& wo) const override;
	vec3 sample_f(vec3& wi, const vec3& wo, float& pdf) const override;
	float pdf(const vec3& wi, const vec3& wo) const override;

private:
	float m_etaI;
	float m_etaT;
	std::shared_ptr<const MicrofacetDistribution> m_distribution;
	std::shared_ptr<const Fresnel> m_fresnel;
};

};

#endif //#ifndef RURT_BTDF_MICROFACET_H