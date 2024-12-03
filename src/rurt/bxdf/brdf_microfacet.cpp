#include "rurt/bxdf/brdf_microfacet.hpp"
#include "rurt/globals.hpp"

//-------------------------------------------//

namespace rurt
{

BRDFMicrofacet::BRDFMicrofacet(std::shared_ptr<const MicrofacetDistribution> distribution, std::shared_ptr<const Fresnel> fresnel) :
	BXDF(false, BXDFType::REFLECTION), m_distribution(distribution), m_fresnel(fresnel)
{

}

vec3 BRDFMicrofacet::f(const vec3& wi, const vec3& wo) const
{
    float cosThetaO = std::abs(cos_theta(wo));
	float cosThetaI = std::abs(cos_theta(wi));
    vec3 wh = wi + wo;

	if(cosThetaI == 0.0f || cosThetaO == 0.0f || (wh.x == 0.0f && wh.y == 0.0f && wh.z == 0.0f))
		return vec3(0.0f);

    wh = normalize(wh);

	return m_distribution->distribution(wh) * 
		   m_distribution->proportion_visible(wi, wo) *
		   m_fresnel->evaluate(dot(wi, wh)) / 
		   (4.0f * cosThetaI * cosThetaO);
}

vec3 BRDFMicrofacet::sample_f(vec3& wi, const vec3& wo, float& pdfVal) const
{
	//TODO

	wi = RURT_UP_DIR;
	pdfVal = 0.0f;
	return vec3(0.0f);
}

float BRDFMicrofacet::pdf(const vec3& wi, const vec3& wo) const
{
	if(!same_hemisphere(wi, wo))
		return 0.0f;

	vec3 wh = normalize(wi + wo);
	return m_distribution->pdf(wo, wh) / (4.0f * dot(wo, wh));
}

}; //namespace rurt