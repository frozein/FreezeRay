#include "rurt/material/material_plastic.hpp"

//-------------------------------------------//

#define ETA_I 1.0f //refractive index of air
#define ETA_T 1.5f //refractive index of glass

namespace rurt
{

MaterialPlastic::MaterialPlastic(const std::string& name, const vec3& colorDiffuse, const vec3& colorSpecular, float roughness) :
	Material(name, false, BXDFType::REFLECTION), m_colorDiffuse(colorDiffuse), m_colorSpecular(colorSpecular),
	m_fresnel(ETA_I, ETA_T), m_distribution(roughness, roughness), m_brdfDiffuse(), 
	m_brdfSpecular(std::make_shared<MicrofacetDistributionTrowbridgeReitz>(m_distribution), std::make_shared<FresnelDielectric>(m_fresnel))
{

}

vec3 MaterialPlastic::bsdf_f(const HitInfo& hitInfo, const vec3& wiWorld, const vec3& woWorld) const
{
	vec3 wi, wo;
	world_to_local(hitInfo.worldNormal, wiWorld, woWorld, wi, wo);

	vec3 f;
	f = f + m_colorDiffuse * m_brdfDiffuse.f(wi, wo);
	f = f + m_colorSpecular * m_brdfSpecular.f(wi, wo);

	return f;
}

vec3 MaterialPlastic::bsdf_sample_f(const HitInfo& hitInfo, vec3& wiWorld, const vec3& woWorld, const vec2& u, float& pdf) const
{
	vec3 wi;
	vec3 wo = world_to_local(hitInfo.worldNormal, woWorld);

	vec3 f = vec3(0.0f);

	if(u.x < 0.5f)
		f = m_colorDiffuse * m_brdfDiffuse.sample_f(wi, wo, pdf);
	else
		f = m_colorSpecular * m_brdfSpecular.sample_f(wi, wo, pdf);

	pdf /= 2.0f;

	wiWorld = local_to_world(hitInfo.worldNormal, wi);
	return f;
}

float MaterialPlastic::bsdf_pdf(const HitInfo& hitInfo, const vec3& wiWorld, const vec3& woWorld) const
{
	vec3 wi, wo;
	world_to_local(hitInfo.worldNormal, wiWorld, woWorld, wi, wo);

	float pdf = 0.0f;
	pdf += m_brdfDiffuse.pdf(wi, wo);
	pdf += m_brdfSpecular.pdf(wi, wo);

	return pdf / 2.0f;
}

}; //namespace rurt