#include "rurt/microfacet_distribution/microfacet_distribution_trowbridge_reitz.hpp"
#include "rurt/globals.hpp"

//-------------------------------------------//

namespace rurt
{

MicrofacetDistributionTrowbridgeReitz::MicrofacetDistributionTrowbridgeReitz(float roughnessX, float roughnessY) :
	m_alphaX(1.0f), m_alphaY(1.0f)
{
	if(roughnessX < 0.0f || roughnessX > 1.0f || roughnessY < 0.0f || roughnessY > 1.0f)
		throw std::invalid_argument("roughness values must be in the range [0.0, 1.0]");

	m_alphaX = roughness_to_alpha(roughnessX);
	m_alphaY = roughness_to_alpha(roughnessY);
}

float MicrofacetDistributionTrowbridgeReitz::distribution(const vec3& w) const
{
	float tanTheta2 = tan_theta2(w);
	if(std::isinf(tanTheta2))
		return 0.0f;

	float e = (cos_phi2(w) / (m_alphaX * m_alphaX) + sin_phi2(w) / (m_alphaY * m_alphaY)) * tanTheta2;
	float cosTheta4 = cos_theta2(w) * cos_theta2(w);
	return 1.0f / (RURT_PI * m_alphaX * m_alphaY * cosTheta4 * (1.0f + e) * (1.0f + e));
}

float MicrofacetDistributionTrowbridgeReitz::invisible_masked_proportion(const vec3& w) const
{
	float absTanTheta = std::abs(tan_theta(w));
	if(std::isinf(absTanTheta))
		return 0.0f;

	float alpha2 = cos_phi2(w) * m_alphaX * m_alphaX + sin_phi2(w) * m_alphaY * m_alphaY;

    float alphaTanTheta2 = alpha2 * absTanTheta * absTanTheta;
    return (-1.0f + std::sqrt(1.0f + alphaTanTheta2)) / 2.0f;	
}

float MicrofacetDistributionTrowbridgeReitz::roughness_to_alpha(float roughness)
{	
	return roughness * roughness;
}

}; //namespace rurt