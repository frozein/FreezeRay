#include "rurt/microfacet_distribution/microfacet_distribution_beckmann.hpp"
#include "rurt/globals.hpp"

//-------------------------------------------//

namespace rurt
{

MicrofacetDistributionBeckmann::MicrofacetDistributionBeckmann(float roughnessX, float roughnessY) :
	m_alphaX(1.0f), m_alphaY(1.0f)
{
	if(roughnessX < 0.0f || roughnessX > 1.0f || roughnessY < 0.0f || roughnessY > 1.0f)
		throw std::invalid_argument("roughness values must be in the range [0.0, 1.0]");

	m_alphaX = roughness_to_alpha(roughnessX);
	m_alphaY = roughness_to_alpha(roughnessY);
}

float MicrofacetDistributionBeckmann::distribution(const vec3& w) const
{
	float tanTheta2 = tan_theta2(w);
	if(std::isinf(tanTheta2))
		return 0.0f;

	float numer = std::exp(-tanTheta2 * (cos_phi2(w) / (m_alphaX * m_alphaX) + sin_phi2(w) / (m_alphaY * m_alphaY)));
	float denom = RURT_PI * m_alphaX * m_alphaY * cos_theta2(w) * cos_theta2(w);

	return numer / denom;
}

float MicrofacetDistributionBeckmann::invisible_masked_proportion(const vec3& w) const
{
	float absTanTheta = std::abs(tan_theta(w));
	if(std::isinf(absTanTheta))
		return 0.0f;

	float alpha = std::sqrtf(cos_phi2(w) * m_alphaX * m_alphaX + sin_phi2(w) * m_alphaY * m_alphaY);

	float a = 1.0f / (alpha * absTanTheta);
	if(a >= 1.6f)
		return 0.0f;
	else
		return (1 - 1.259f * a + 0.396f * a * a) / (3.535f * a + 2.181f * a * a);
}

float MicrofacetDistributionBeckmann::roughness_to_alpha(float roughness)
{
	return roughness * roughness;
}

}; //namespace rurt