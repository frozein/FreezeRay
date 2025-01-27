#include "freezeray/bxdf/fr_btdf_microfacet.hpp"
#include "freezeray/fr_globals.hpp"

//-------------------------------------------//

namespace fr
{

BTDFMicrofacet::BTDFMicrofacet(float etaI, float etaT, std::shared_ptr<const MicrofacetDistribution> distribution, std::shared_ptr<const Fresnel> fresnel) :
	BXDF(false, BXDFType::TRANSMISSION), m_etaI(etaI), m_etaT(etaT), m_distribution(distribution), m_fresnel(fresnel)
{

}

vec3 BTDFMicrofacet::f(const vec3& wi, const vec3& wo) const
{
	//validate:
	//---------------
    if(same_hemisphere(wo, wi))
	{
		std::cout << "failed 1\n";
		return vec3(0.0f);
	}

    float cosThetaO = cos_theta(wo);
    float cosThetaI = cos_theta(wi);
    if(cosThetaI == 0.0f || cosThetaO == 0.0f) 
		return vec3(0.0f);

	//get indices of refraction:
	//---------------
	bool entering = cosThetaO > 0.0f;

	float etaI;
	float etaT;
	if(entering)
	{
		etaI = m_etaI;
		etaT = m_etaT;
	}
	else //not entering, need to swap
	{
		etaI = m_etaT;
		etaT = m_etaI;
	}

	//get half-vector:
	//---------------
    float eta = m_etaT / m_etaI;
    vec3 wh = normalize(wo + wi * eta);
    if(wh.y < 0.0f) 
		wh = -1.0f * wh;

	//compute f:
	//---------------
    if(dot(wo, wh) * dot(wi, wh) > 0.0f) 
	{
		std::cout << "failed 2\n";
		return vec3(0.0f);
	}

    float sqrtDenom = dot(wo, wh) + eta * dot(wi, wh);

    return (1.0f - m_fresnel->evaluate(dot(wo, wh))) * 
	       std::abs(
	                m_distribution->distribution(wh) * 
					m_distribution->proportion_visible(wo, wi) * 
					eta * eta *
                    std::abs(dot(wi, wh)) * std::abs(dot(wo, wh)) /
                    (cosThetaI * cosThetaO * sqrtDenom * sqrtDenom)
			);
}

vec3 BTDFMicrofacet::sample_f(vec3& wi, const vec3& wo, const vec2& u, float& pdfVal) const
{
	//sample wh:
	//---------------
    vec3 wh = m_distribution->sample_distribution(wo, u);

	//refract:
	//---------------
    float eta = cos_theta(wo) > 0.0f ? (m_etaI / m_etaT) : (m_etaT / m_etaI);

	float cosThetaI = dot(wh, wi);
	float sinTheta2I = std::max(0.0f, 1.0f - cosThetaI * cosThetaI);
	float sinTheta2T = eta * eta * sinTheta2I;

	if(sinTheta2T >= 1.0f)
	{
		pdfVal = 1.0f;
		return vec3(0.0f);
	}

	float cosThetaT = std::sqrt(1.0f - sinTheta2T);
    wi = -eta * wo + (eta * cosThetaI - cosThetaT) * wh;

	//return:
	//---------------
    pdfVal = pdf(wi, wo);
    return f(wi, wo);
}

float BTDFMicrofacet::pdf(const vec3& wi, const vec3& wo) const
{
    if (same_hemisphere(wo, wi)) 
		return 0.0f;

	bool entering = cos_theta(wo) > 0.0f;

	float etaI;
	float etaT;
	if(entering)
	{
		etaI = m_etaI;
		etaT = m_etaT;
	}
	else //not entering, need to swap
	{
		etaI = m_etaT;
		etaT = m_etaI;
	}

    float eta = etaT / etaI;
    vec3 wh = normalize(wo + wi * eta);

    if(dot(wo, wh) * dot(wi, wh) > 0.0f) 
		return 0;

    float sqrtDenom = dot(wo, wh) + eta * dot(wi, wh);

    return m_distribution->pdf(wo, wh) * std::abs((eta * eta * dot(wi, wh)) / (sqrtDenom * sqrtDenom));
}

}; //namespace fr