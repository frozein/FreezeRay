#include "freezeray/microfacet_distribution/fr_microfacet_distribution_trowbridge_reitz.hpp"
#include "freezeray/fr_globals.hpp"

//-------------------------------------------//

namespace fr
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
	return 1.0f / (FR_PI * m_alphaX * m_alphaY * cosTheta4 * (1.0f + e) * (1.0f + e));
}

vec3 MicrofacetDistributionTrowbridgeReitz::sample_distribution(const vec3& w, const vec2& u) const
{
	float cosTheta;
	float phi;

	if(m_alphaX == m_alphaY)
	{
		float tanTheta2 = m_alphaX * m_alphaX * u.x / (1.0f - u.x);
		
		cosTheta = 1.0f / std::sqrt(1.0f + tanTheta2);
		phi = FR_2_PI * u.y;
	} 
	else 
	{
		phi = std::atan(m_alphaY / m_alphaX * std::tan(FR_2_PI * u.y + 0.5f * FR_PI));
		if(u.y > 0.5f) 
			phi += FR_PI;

		float sinPhi = std::sin(phi);
		float cosPhi = std::cos(phi);
		
		float alphaX2 = m_alphaX * m_alphaX;
		float alphaY2 = m_alphaY * m_alphaY;
		float alpha2 = 1.0f / (cosPhi * cosPhi / alphaX2 + sinPhi * sinPhi / alphaY2);

		float tanTheta2 = alpha2 * u.x / (1 - u.x);
		cosTheta = 1 / std::sqrt(1 + tanTheta2);
	}

	float sinTheta = std::sqrtf(std::max(0.0f, 1.0f - cosTheta * cosTheta));
	
	vec3 wh = vec3(sinTheta * std::cos(phi), cosTheta, sinTheta * std::sin(phi));
	if(!same_hemisphere(w, wh)) 
		wh = -1.0f * wh;

	return wh;
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

}; //namespace fr