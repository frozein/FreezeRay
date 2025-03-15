#include "freezeray/material/fr_material_plastic.hpp"
#include "freezeray/bxdf/fr_brdf_microfacet.hpp"
#include "freezeray/microfacet_distribution/fr_microfacet_distribution_trowbridge_reitz.hpp"

//-------------------------------------------//

namespace fr
{

#define ETA_I 1.0f //refractive index of air
#define ETA_T 1.5f //refractive index of glass

//-------------------------------------------//

MaterialPlastic::MaterialPlastic(const std::string& name, const std::shared_ptr<Texture<vec3>>& colorDiffuse, const std::shared_ptr<Texture<vec3>>& colorSpecular, 
	                             const std::shared_ptr<Texture<float>>& roughness) :
	Material(name), m_colorDiffuse(colorDiffuse), m_colorSpecular(colorSpecular), m_roughness(roughness)
{

}

std::shared_ptr<BSDF> MaterialPlastic::get_bsdf(const IntersectionInfo& hitInfo) const
{
	//create distribution + fresnel:
	//---------------
	float roughness = m_roughness->evaluate(hitInfo);
	std::shared_ptr<MicrofacetDistribution> distribution = 
		std::make_shared<MicrofacetDistributionTrowbridgeReitz>(roughness, roughness);

	std::shared_ptr<Fresnel> fresnel =
		std::make_shared<FresnelDielectric>(ETA_I, ETA_T);

	//create bsdf:
	//---------------
	std::shared_ptr<BSDF> bsdf = std::make_shared<BSDF>(hitInfo.worldNormal);

	bsdf->add_bxdf(
		std::make_shared<BRDFLambertian>(),
		m_colorDiffuse->evaluate(hitInfo)
	);

	bsdf->add_bxdf(
		std::make_shared<BRDFMicrofacet>(distribution, fresnel),
		m_colorSpecular->evaluate(hitInfo)
	);

	return bsdf;
}

}; //namespace fr