#include "freezeray/material/fr_material_mirror.hpp"

//-------------------------------------------//

namespace fr
{

MaterialMirror::MaterialMirror(const std::string& name, const std::shared_ptr<Texture<vec3>>& color) : 
	Material(name), m_color(color) 
{

}

std::shared_ptr<BSDF> MaterialMirror::get_bsdf(const IntersectionInfo& hitInfo) const
{
	//create fresnel:
	//---------------
	std::shared_ptr<Fresnel> fresnel =
		std::make_shared<FresnelConstant>(1.0f);

	//create BSDF:
	//---------------
	std::shared_ptr<BSDF> bsdf = std::make_shared<BSDF>(hitInfo.worldNormal);

	bsdf->add_bxdf(
		std::make_shared<BRDFSpecular>(fresnel), 
		m_color->evaluate(hitInfo)
	);

	return bsdf;
}

}; //namespace fr