#include "rurt/material/material_specular_glass.hpp"
#include "rurt/globals.hpp"

#define ETA_I 1.0f //refractive index of air

//-------------------------------------------//

namespace rurt
{

MaterialSpecularGlass::MaterialSpecularGlass(const std::string& name, float eta, const std::shared_ptr<Texture<vec3>>& colorReflection, const std::shared_ptr<Texture<vec3>>& colorTransmission) : 
	Material(name, true, BXDFType::BOTH), m_etaT(eta), m_colorReflection(colorReflection), m_colorTransmission(colorTransmission), m_fresnel(ETA_I, eta),
	m_brdf(std::make_shared<FresnelDielectric>(m_fresnel)), m_btdf(ETA_I, eta, std::make_shared<FresnelDielectric>(m_fresnel))
{

}

vec3 MaterialSpecularGlass::bsdf_f(const IntersectionInfo& hitInfo, const vec3& wiWorld, const vec3& woWorld) const
{
	//delta distribution
	return vec3(0.0f);
}

vec3 MaterialSpecularGlass::bsdf_sample_f(const IntersectionInfo& hitInfo, vec3& wiWorld, const vec3& woWorld, const vec2& u, float& pdf) const
{
	vec3 wi;
	vec3 wo = world_to_local(hitInfo.worldNormal, woWorld);
    
	vec3 f = vec3(0.0f);

    if(u.x < 0.5f)
        f = m_colorReflection->evaluate(hitInfo) * m_brdf.sample_f(wi, wo, pdf);
	else
	{
		bool entering = cos_theta(wo) > 0.0f;
		float eta;
		if(entering)
			eta = ETA_I / m_etaT;
		else
			eta = m_etaT / ETA_I;

		float cosThetaI = std::abs(cos_theta(wo));
        float sinTheta2T = eta * eta * (1.0f - cosThetaI * cosThetaI);
        if(sinTheta2T >= 1.0f) //total internal reflection
            f = m_colorReflection->evaluate(hitInfo) * m_brdf.sample_f(wi, wo, pdf);
		else
            f = m_colorTransmission->evaluate(hitInfo) * m_btdf.sample_f(wi, wo, pdf);
    }

	pdf /= 2.0f;

	wiWorld = local_to_world(hitInfo.worldNormal, wi);
	return f;
}

float MaterialSpecularGlass::bsdf_pdf(const IntersectionInfo& hitInfo, const vec3& wiWorld, const vec3& woWorld) const
{
	//delta distribution
	return 1.0f;
}

}; //namespace rurt