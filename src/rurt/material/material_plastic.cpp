#include "rurt/material/material_plastic.hpp"
#include "rurt/bxdf/brdf_microfacet.hpp"
#include "rurt/microfacet_distribution/microfacet_distribution_trowbridge_reitz.hpp"

//-------------------------------------------//

namespace rurt
{

#define ETA_I 1.0f //refractive index of air
#define ETA_T 1.5f //refractive index of glass

#define MAKE_MICROFACET_DISTRIBUTION()                \
	float roughness = m_roughness->evaluate(hitInfo); \
	MicrofacetDistributionTrowbridgeReitz distribution(roughness, roughness);

#define MAKE_BRDF_SPECULAR()       \
	MAKE_MICROFACET_DISTRIBUTION() \
	BRDFMicrofacet brdfSpecular = BRDFMicrofacet(std::shared_ptr<const MicrofacetDistribution>(&distribution), std::shared_ptr<const Fresnel>(&m_fresnel));

//-------------------------------------------//

MaterialPlastic::MaterialPlastic(const std::string& name, const std::shared_ptr<Texture<vec3>>& colorDiffuse, const std::shared_ptr<Texture<vec3>>& colorSpecular, 
	                             const std::shared_ptr<Texture<float>>& roughness) :
	Material(name, false, BXDFType::REFLECTION), m_colorDiffuse(colorDiffuse), m_colorSpecular(colorSpecular), m_roughness(roughness), 
	m_brdfDiffuse(), m_fresnel(ETA_I, ETA_T)
{

}

vec3 MaterialPlastic::bsdf_f(const IntersectionInfo& hitInfo, const vec3& wiWorld, const vec3& woWorld) const
{
	vec3 wi, wo;
	world_to_local(hitInfo.worldNormal, wiWorld, woWorld, wi, wo);

	MAKE_BRDF_SPECULAR();

	vec3 f;
	f = f + m_colorDiffuse->evaluate(hitInfo) * m_brdfDiffuse.f(wi, wo);
	f = f + m_colorSpecular->evaluate(hitInfo) * brdfSpecular.f(wi, wo);

	return f;
}

vec3 MaterialPlastic::bsdf_sample_f(const IntersectionInfo& hitInfo, vec3& wiWorld, const vec3& woWorld, const vec2& u, float& pdf) const
{
	vec3 wi;
	vec3 wo = world_to_local(hitInfo.worldNormal, woWorld);

	vec3 f = vec3(0.0f);

	if(u.x < 0.5f)
		f = m_colorDiffuse->evaluate(hitInfo) * m_brdfDiffuse.sample_f(wi, wo, pdf);
	else
	{
		MAKE_BRDF_SPECULAR();
		f = m_colorSpecular->evaluate(hitInfo) * brdfSpecular.sample_f(wi, wo, pdf);
	}

	pdf /= 2.0f;

	wiWorld = local_to_world(hitInfo.worldNormal, wi);
	return f;
}

float MaterialPlastic::bsdf_pdf(const IntersectionInfo& hitInfo, const vec3& wiWorld, const vec3& woWorld) const
{
	vec3 wi, wo;
	world_to_local(hitInfo.worldNormal, wiWorld, woWorld, wi, wo);

	MAKE_BRDF_SPECULAR();

	float pdf = 0.0f;
	pdf += m_brdfDiffuse.pdf(wi, wo);
	pdf += brdfSpecular.pdf(wi, wo);

	return pdf / 2.0f;
}

}; //namespace rurt