#include "rurt/material/material_specular_glass.hpp"
#include "rurt/globals.hpp"

#define ETA_I 1.0f //refractive index of air
#define ETA_T 1.5f //refractive index of glass

//-------------------------------------------//

namespace rurt
{

MaterialSpecularGlass::MaterialSpecularGlass(const std::string& name, const vec3& color) : 
	Material(name, true, BXDFType::BOTH), m_color(color), m_fresnel(ETA_I, ETA_T),
	m_brdf(std::make_shared<FresnelDielectric>(m_fresnel)), m_btdf(ETA_I, ETA_T, std::make_shared<FresnelDielectric>(m_fresnel))
{

}

vec3 MaterialSpecularGlass::bsdf_f(const HitInfo& hitInfo, const vec3& wiWorld, const vec3& woWorld) const
{
	//delta distribution
	return vec3(0.0f);
}

vec3 MaterialSpecularGlass::bsdf_sample_f(const HitInfo& hitInfo, vec3& wiWorld, const vec3& woWorld, const vec2& u, float& pdf) const
{
	vec3 wi;
	vec3 wo = world_to_local(hitInfo.worldNormal, woWorld);
    
	vec3 f;

    if(u.x < 0.5f)
        f = m_brdf.sample_f(wi, wo, pdf);
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
            f = m_brdf.sample_f(wi, wo, pdf);
		else
            f = m_btdf.sample_f(wi, wo, pdf);
    }

	pdf /= 2.0f;

	wiWorld = local_to_world(hitInfo.worldNormal, wi);
	return m_color * f;
}

float MaterialSpecularGlass::bsdf_pdf(const HitInfo& hitInfo, const vec3& wiWorld, const vec3& woWorld) const
{
	//delta distribution
	return 1.0f;
}

}; //namespace rurt