#include "freezeray/material/fr_material_glass.hpp"
#include "freezeray/fr_globals.hpp"
#include "freezeray/bxdf/fr_brdf_microfacet.hpp"
#include "freezeray/bxdf/fr_btdf_microfacet.hpp"
#include "freezeray/microfacet_distribution/fr_microfacet_distribution_trowbridge_reitz.hpp"

//-------------------------------------------//

namespace fr
{

#define ETA_I 1.0f //refractive index of air

#define MAKE_MICROFACET_DISTRIBUTION()                  \
	float roughnessX = m_roughnessX->evaluate(hitInfo); \
	float roughnessY = m_roughnessY->evaluate(hitInfo); \
	MicrofacetDistributionTrowbridgeReitz distribution(roughnessX, roughnessY);

#define MAKE_BRDF() \
	BRDFMicrofacet brdf = BRDFMicrofacet(std::shared_ptr<const MicrofacetDistribution>(&distribution), std::shared_ptr<const Fresnel>(&m_fresnel));

#define MAKE_BTDF() \
	BTDFMicrofacet btdf(1.0f, m_etaT, std::shared_ptr<const MicrofacetDistribution>(&distribution), std::shared_ptr<const Fresnel>(&m_fresnel));		

//-------------------------------------------//

MaterialGlass::MaterialGlass(const std::string& name, float eta, const std::shared_ptr<Texture<vec3>>& colorReflection, const std::shared_ptr<Texture<vec3>>& colorTransmission, 
                             const std::shared_ptr<Texture<float>>& roughnessX, const std::shared_ptr<Texture<float>>& roughnessY) : 
	Material(name, false, BXDFType::BOTH), m_etaT(eta), m_colorReflection(colorReflection), m_colorTransmission(colorTransmission), 
	m_roughnessX(roughnessX), m_roughnessY(roughnessY), m_fresnel(ETA_I, eta)
{

}

vec3 MaterialGlass::bsdf_f(const IntersectionInfo& hitInfo, const vec3& wiWorld, const vec3& woWorld) const
{
	vec3 wi, wo;
	world_to_local(hitInfo.worldNormal, wiWorld, woWorld, wi, wo);

	MAKE_MICROFACET_DISTRIBUTION();

	vec3 f;
	if(dot(wi, wo) >= 0.0f) //reflection
	{
		MAKE_BRDF();
		f = m_colorReflection->evaluate(hitInfo) * brdf.f(wi, wo);
	}
	else //transmission
	{
		MAKE_BTDF();
		f = m_colorReflection->evaluate(hitInfo) * btdf.f(wi, wo);
	}

	return f;
}

vec3 MaterialGlass::bsdf_sample_f(const IntersectionInfo& hitInfo, vec3& wiWorld, const vec3& woWorld, const vec3& u, float& pdf) const
{
	vec3 wi;
	vec3 wo = world_to_local(hitInfo.worldNormal, woWorld);
    
	vec3 f = vec3(0.0f);

	MAKE_MICROFACET_DISTRIBUTION();

    if(u.x < 0.5f)
	{
		MAKE_BRDF();
        f = m_colorReflection->evaluate(hitInfo) * brdf.sample_f(wi, wo, u.yz(), pdf);
	}
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
		{
			MAKE_BRDF();
			f = m_colorReflection->evaluate(hitInfo) * brdf.sample_f(wi, wo, u.yz(), pdf);
		}
		else
		{
			MAKE_BTDF();
			f = m_colorReflection->evaluate(hitInfo) * btdf.sample_f(wi, wo, u.yz(), pdf);
		}
    }

	pdf /= 2.0f;

	wiWorld = local_to_world(hitInfo.worldNormal, wi);
	return f;
}

float MaterialGlass::bsdf_pdf(const IntersectionInfo& hitInfo, const vec3& wiWorld, const vec3& woWorld) const
{
	vec3 wi, wo;
	world_to_local(hitInfo.worldNormal, wiWorld, woWorld, wi, wo);

	MAKE_MICROFACET_DISTRIBUTION();

	float pdf;
	if(dot(wi, wo) >= 0.0f) //reflection
	{
		MAKE_BRDF();
		pdf = brdf.pdf(wi, wo);
	}
	else //transmission
	{
		MAKE_BTDF();
		pdf = btdf.pdf(wi, wo);
	}
 
	return pdf / 2.0f;
}

}; //namespace fr