#include "freezeray/bxdf/fr_brdf_microfacet.hpp"
#include "freezeray/fr_globals.hpp"

//-------------------------------------------//

namespace fr
{

BRDFMicrofacet::BRDFMicrofacet(std::shared_ptr<const MicrofacetDistribution> distribution, std::shared_ptr<const Fresnel> fresnel) :
	BXDF(BXDFflags::REFLECTION), m_distribution(distribution), m_fresnel(fresnel)
{

}

vec3 BRDFMicrofacet::f(const vec3& wi, const vec3& wo) const
{
	if(!same_hemisphere(wi, wo))
		return 0.0f;

    float cosThetaO = std::abs(cos_theta(wo));
	float cosThetaI = std::abs(cos_theta(wi));
    vec3 wh = wi + wo;

	if(cosThetaI == 0.0f || cosThetaO == 0.0f || wh == vec3(0.0f))
		return vec3(0.0f);

    wh = normalize(wh);

	return m_distribution->distribution(wh) * 
		   m_distribution->proportion_visible(wi, wo) *
		   m_fresnel->evaluate(dot(wi, wh)) / 
		   (4.0f * cosThetaI * cosThetaO);
}

vec3 BRDFMicrofacet::sample_f(vec3& wi, const vec3& wo, const vec2& u, float& pdfVal) const
{
	if(wo.y < 0.0f)
		return vec3(0.0f);

	vec3 wh = m_distribution->sample_distribution(wo, u);
	if(dot(wo, wh) < 0.0f)
		return vec3(0.0f);

	wi = -1.0f * wo + 2.0f * dot(wo, wh) * wh;
	if(!same_hemisphere(wo, wi))
		return vec3(0.0f);

	pdfVal = m_distribution->pdf(wo, wh) / (4.0f * dot(wo, wh));
	return f(wi, wo);
}

float BRDFMicrofacet::pdf(const vec3& wi, const vec3& wo) const
{
	if(!same_hemisphere(wi, wo))
		return 0.0f;

	vec3 wh = normalize(wi + wo);
	return m_distribution->pdf(wo, wh) / (4.0f * dot(wo, wh));
}

}; //namespace fr