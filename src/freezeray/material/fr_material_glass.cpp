#include "freezeray/material/fr_material_glass.hpp"
#include "freezeray/fr_globals.hpp"
#include "freezeray/bxdf/fr_brdf_microfacet.hpp"
#include "freezeray/bxdf/fr_btdf_microfacet.hpp"
#include "freezeray/microfacet_distribution/fr_microfacet_distribution_trowbridge_reitz.hpp"

//-------------------------------------------//

namespace fr
{

#define ETA_I 1.0f //refractive index of air	

//-------------------------------------------//

MaterialGlass::MaterialGlass(const std::string& name, float eta, const std::shared_ptr<Texture<vec3>>& colorReflection, const std::shared_ptr<Texture<vec3>>& colorTransmission, 
                             const std::shared_ptr<Texture<float>>& roughnessX, const std::shared_ptr<Texture<float>>& roughnessY) : 
	Material(name), m_etaT(eta), m_colorReflection(colorReflection), m_colorTransmission(colorTransmission), 
	m_roughnessX(roughnessX), m_roughnessY(roughnessY)
{

}

std::shared_ptr<BSDF> MaterialGlass::get_bsdf(const IntersectionInfo& hitInfo) const
{
	//create distribution + fresnel:
	//---------------
	float roughnessX = m_roughnessX->evaluate(hitInfo);
	float roughnessY = m_roughnessY->evaluate(hitInfo);
	std::shared_ptr<MicrofacetDistribution> distribution = 
		std::make_shared<MicrofacetDistributionTrowbridgeReitz>(roughnessX, roughnessY);

	std::shared_ptr<Fresnel> fresnel =
		std::make_shared<FresnelDielectric>(ETA_I, m_etaT);

	//create bsdf:
	//---------------
	std::shared_ptr<BSDF> bsdf = std::make_shared<BSDF>(hitInfo.shadingNormal);

	bsdf->add_bxdf(
		std::make_shared<BRDFMicrofacet>(distribution, fresnel), 
		m_colorReflection->evaluate(hitInfo)
	);

	bsdf->add_bxdf(
		std::make_shared<BTDFMicrofacet>(ETA_I, m_etaT, distribution, fresnel), 
		m_colorTransmission->evaluate(hitInfo)
	);

	return bsdf;
}

}; //namespace fr