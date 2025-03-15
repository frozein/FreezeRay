#include "freezeray/material/fr_material_metal.hpp"
#include <unordered_map>
#include "freezeray/bxdf/fr_brdf_microfacet.hpp"
#include "freezeray/microfacet_distribution/fr_microfacet_distribution_trowbridge_reitz.hpp"
#include "freezeray/fresnel/fr_fresnel_conductor.hpp"
#include "freezeray/texture/fr_texture_constant.hpp"

//-------------------------------------------//

namespace fr
{

#define ETA_I vec3(1.0f) //refractive index of air

//-------------------------------------------//

struct MetalParams
{
	vec3 eta;
	vec3 absorption;
};

const std::unordered_map<MetalType, const MetalParams> PARAMS = {
	{ MetalType::ALUMINUM, { vec3(1.34560f, 0.96521f, 0.61722f), vec3(7.47460f, 6.39950f, 5.30310f) } },
	{ MetalType::BRASS,    { vec3(0.44400f, 0.52700f, 1.09400f), vec3(3.69500f, 2.76500f, 1.82900f) } },
	{ MetalType::COPPER,   { vec3(0.27105f, 0.67693f, 1.31640f), vec3(3.60920f, 2.62480f, 2.29210f) } },
	{ MetalType::GOLD,     { vec3(0.18299f, 0.42108f, 1.37340f), vec3(3.42420f, 2.34590f, 1.77040f) } },
	{ MetalType::IRON,     { vec3(2.91140f, 2.94970f, 2.58450f), vec3(3.08930f, 2.93180f, 2.76700f) } },
	{ MetalType::LEAD,     { vec3(1.91000f, 1.83000f, 1.44000f), vec3(3.51000f, 3.40000f, 3.18000f) } },
	{ MetalType::MERCURY,  { vec3(2.07330f, 1.55230f, 1.06060f), vec3(5.33830f, 4.65100f, 3.86280f) } },
	{ MetalType::PLATINUM, { vec3(2.37570f,	2.08470f, 1.84530f), vec3(4.26550f, 3.71530f, 3.13650f) } },
	{ MetalType::SILVER,   { vec3(0.15943f, 0.14512f, 0.13547f), vec3(3.92910f, 3.19000f, 2.38080f) } },
	{ MetalType::TITANIUM, { vec3(2.74070f, 2.54180f, 2.26700f), vec3(3.81430f, 3.43450f, 3.03850f) } }
};

//-------------------------------------------//

MaterialMetal::MaterialMetal(const std::string& name, const std::shared_ptr<Texture<vec3>>& eta, const std::shared_ptr<Texture<vec3>>& k, 
                             const std::shared_ptr<Texture<float>>& roughnessU, const std::shared_ptr<Texture<float>>& roughnessV) :
	Material(name), m_etaT(eta), m_absorption(k), m_roughnessX(roughnessU), m_roughnessY(roughnessV)
{

}

MaterialMetal::MaterialMetal(const std::string& name, const MetalType& type, const std::shared_ptr<Texture<float>>& roughnessU, const std::shared_ptr<Texture<float>>& roughnessV) :
	Material(name), m_etaT(std::make_shared<TextureConstant<vec3>>(PARAMS.at(type).eta)),
	m_absorption(std::make_shared<TextureConstant<vec3>>(PARAMS.at(type).absorption)), m_roughnessX(roughnessU), m_roughnessY(roughnessV)
{

}

std::shared_ptr<BSDF> MaterialMetal::get_bsdf(const IntersectionInfo& hitInfo) const
{
	//create distribution + fresnel:
	//---------------
	float roughnessX = m_roughnessX->evaluate(hitInfo);
	float roughnessY = m_roughnessY->evaluate(hitInfo);
	std::shared_ptr<MicrofacetDistribution> distribution =
		std::make_shared<MicrofacetDistributionTrowbridgeReitz>(roughnessX, roughnessY);

	vec3 etaT = m_etaT->evaluate(hitInfo);
	vec3 absorption = m_absorption->evaluate(hitInfo);
	std::shared_ptr<Fresnel> fresnel = 
		std::make_shared<FresnelConductor>(ETA_I, etaT, absorption);

	//create bsdf:
	//---------------
	std::shared_ptr<BSDF> bsdf = std::make_shared<BSDF>(hitInfo.worldNormal);

	bsdf->add_bxdf(
		std::make_shared<BRDFMicrofacet>(distribution, fresnel),
		vec3(1.0f)
	);

	return bsdf;
}

}; //namespace fr