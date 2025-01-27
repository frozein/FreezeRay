#include "freezeray/microfacet_distribution/fr_microfacet_distribution_beckmann.hpp"
#include "freezeray/fr_globals.hpp"

//-------------------------------------------//

namespace fr
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
	float denom = FR_PI * m_alphaX * m_alphaY * cos_theta2(w) * cos_theta2(w);

	return numer / denom;
}

vec3 MicrofacetDistributionBeckmann::sample_distribution(const vec3& w, const vec2& u) const
{
	float tanTheta2; 
	float phi;

	if(m_alphaX == m_alphaY) 
	{
		float logSample = std::log(std::max(1 - u.x, 1e-10f));
		
		tanTheta2 = -m_alphaX * m_alphaX * logSample;
		phi = u.y * FR_2_PI;
	} 
	else 
	{
		float logSample = std::log(std::max(1 - u.x, 1e-10f));
		
		phi = std::atan(m_alphaY / m_alphaX * std::tan(FR_2_PI * u.y + 0.5f * FR_PI));
		if(u.y > 0.5f) 
			phi += FR_PI;
		
		float sinPhi = std::sin(phi);
		float cosPhi = std::cos(phi);

		float alphaX2 = m_alphaX * m_alphaX;
		float alphaY2 = m_alphaY * m_alphaY;
		
		tanTheta2 = -logSample / (cosPhi * cosPhi / alphaX2 + sinPhi * sinPhi / alphaY2);
	}

	float cosTheta = 1.0f / std::sqrt(1.0f + tanTheta2);
	float sinTheta = std::sqrtf(std::max(0.0f, 1.0f - cosTheta * cosTheta));

	vec3 wh = vec3(sinTheta * std::cos(phi), cosTheta, sinTheta * std::sin(phi));
	if(!same_hemisphere(w, wh)) 
		wh = -1.0f * wh;
	
	return wh;
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

}; //namespace fr