#include "freezeray/material/fr_material_specular_glass.hpp"
#include "freezeray/fr_globals.hpp"

#define ETA_I 1.0f //refractive index of air

//-------------------------------------------//

namespace fr
{

MaterialSpecularGlass::MaterialSpecularGlass(const std::string& name, float eta, const std::shared_ptr<Texture<vec3>>& colorReflection, const std::shared_ptr<Texture<vec3>>& colorTransmission) : 
	Material(name), m_etaT(eta), m_colorReflection(colorReflection), m_colorTransmission(colorTransmission)
{

}

std::shared_ptr<BSDF> MaterialSpecularGlass::get_bsdf(const IntersectionInfo& hitInfo) const
{
	//create distribution + fresnel:
	//---------------
	std::shared_ptr<Fresnel> fresnel =
		std::make_shared<FresnelDielectric>(ETA_I, m_etaT);

	//create bsdf:
	//---------------
	std::shared_ptr<BSDF> bsdf = std::make_shared<BSDF>(hitInfo.worldNormal);

	bsdf->add_bxdf(
		std::make_shared<BRDFSpecular>(fresnel), 
		m_colorReflection->evaluate(hitInfo)
	);

	bsdf->add_bxdf(
		std::make_shared<BTDFSpecular>(ETA_I, m_etaT, fresnel), 
		m_colorTransmission->evaluate(hitInfo)
	);

	return bsdf;
}

}; //namespace fr