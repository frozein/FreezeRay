#include "rurt/brdf/microfacet_distribution/microfacet_distribution_beckmann.hpp"
#include "rurt/constants.hpp"

//-------------------------------------------//

namespace rurt
{

MicrofacetDistributionBeckmann::MicrofacetDistributionBeckmann(float roughnessX, float roughnessY) :
	m_alphaX(roughness_to_alpha(roughnessX)), m_alphaY(roughness_to_alpha(roughnessY))
{

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
	roughness = std::max(roughness, RURT_EPSILON);
	float x = std::log(roughness);
	return 1.62142f + 0.819955f * x + 0.1734f * x * x + 0.0171201f * x * x * x + 0.000640711f * x * x * x * x;
}

}; //namespace rurt