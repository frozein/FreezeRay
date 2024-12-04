#include "rurt/material/material_glass.hpp"
#include "rurt/globals.hpp"

#define ETA_I 1.0f //refractive index of air
#define ETA_T 1.5f //refractive index of glass

//-------------------------------------------//

namespace rurt
{

MaterialGlass::MaterialGlass(const std::string& name, const vec3& colorReflection, const vec3& colorTransmission, float roughnessU, float roughnessV) : 
	Material(name, false, BXDFType::BOTH), m_colorReflection(colorReflection), m_colorTransmission(colorTransmission), 
	m_fresnel(ETA_I, ETA_T), m_distribution(roughnessU, roughnessV), 
	m_brdf(std::make_shared<MicrofacetDistributionTrowbridgeReitz>(m_distribution), std::make_shared<FresnelDielectric>(m_fresnel)),
	m_btdf(ETA_I, ETA_T, std::make_shared<MicrofacetDistributionTrowbridgeReitz>(m_distribution), std::make_shared<FresnelDielectric>(m_fresnel))
{

}

vec3 MaterialGlass::bsdf_f(const HitInfo& hitInfo, const vec3& wiWorld, const vec3& woWorld) const
{
	vec3 wi, wo;
	world_to_local(hitInfo.worldNormal, wiWorld, woWorld, wi, wo);

	vec3 f;
	if(dot(wi, wo) >= 0.0f) //reflection
		f = m_colorReflection * m_brdf.f(wi, wo);
	else //transmission
		f = m_colorTransmission * m_btdf.f(wi, wo);

	return f;
}

vec3 MaterialGlass::bsdf_sample_f(const HitInfo& hitInfo, vec3& wiWorld, const vec3& woWorld, const vec2& u, float& pdf) const
{
	vec3 wi;
	vec3 wo = world_to_local(hitInfo.worldNormal, woWorld);
    
	vec3 f = vec3(0.0f);

    if(u.x < 0.5f)
        f = m_colorReflection * m_brdf.sample_f(wi, wo, pdf);
	else
	{
		bool entering = cos_theta(wo) > 0.0f;
		float eta;
		if(entering)
			eta = ETA_I / ETA_T;
		else
			eta = ETA_T / ETA_I;

		float cosThetaI = std::abs(cos_theta(wo));
        float sinTheta2T = eta * eta * (1.0f - cosThetaI * cosThetaI);
        if(sinTheta2T >= 1.0f) //total internal reflection
            f = m_colorReflection * m_brdf.sample_f(wi, wo, pdf);
		else
            f = m_colorTransmission * m_btdf.sample_f(wi, wo, pdf);
    }

	pdf /= 2.0f;

	wiWorld = local_to_world(hitInfo.worldNormal, wi);
	return f;
}

float MaterialGlass::bsdf_pdf(const HitInfo& hitInfo, const vec3& wiWorld, const vec3& woWorld) const
{
	vec3 wi, wo;
	world_to_local(hitInfo.worldNormal, wiWorld, woWorld, wi, wo);

	float pdf;
	if(dot(wi, wo) >= 0.0f) //reflection
		pdf = m_brdf.pdf(wi, wo);
	else //transmission
		pdf = m_btdf.pdf(wi, wo);

	return pdf / 2.0f;
}

}; //namespace rurt