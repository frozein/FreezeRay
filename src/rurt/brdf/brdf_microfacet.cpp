#include "rurt/brdf/brdf_microfacet.hpp"
#include "rurt/globals.hpp"

//-------------------------------------------//

namespace rurt
{

BRDFMicrofacet::BRDFMicrofacet(const vec3& color, std::shared_ptr<const MicrofacetDistribution> distribution) :
	m_color(srgb_to_linear(color)), m_distribution(distribution)
{

}

vec3 BRDFMicrofacet::f(const HitInfo& info, const vec3& wi, const vec3& wo) const
{
    float cosThetaO = std::abs(cos_theta(wo));
	float cosThetaI = std::abs(cos_theta(wi));
    vec3 wh = wi + wo;

	if(cosThetaI == 0.0f || cosThetaO == 0.0f || wh.x == 0.0f && wh.y == 0.0f && wh.z == 0.0f)
		return vec3(0.0f);

    wh = normalize(wh);

	return m_color * m_distribution->distribution(wh) * m_distribution->proportion_visible(wi, wo) / (4.0f * cosThetaI * cosThetaO);
}

vec3 BRDFMicrofacet::sample_f(const HitInfo& info, vec3& wi, const vec3& wo, float& pdfVal) const
{
	//TODO

	wi = RURT_UP_DIR;
	pdfVal = 0.0f;
	return vec3(0.0f);
}

float BRDFMicrofacet::pdf(const HitInfo& info, const vec3& wi, const vec3& wo) const
{
	if(!same_hemisphere(wi, wo))
		return 0.0f;

	vec3 wh = normalize(wi + wo);
	return m_distribution->pdf(wo, wh) / (4.0f * dot(wo, wh));
}

}; //namespace rurt