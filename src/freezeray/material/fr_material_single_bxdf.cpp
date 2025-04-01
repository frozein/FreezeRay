#include "freezeray/material/fr_material_single_bxdf.hpp"

//-------------------------------------------//

namespace fr
{

MaterialSingleBXDF::MaterialSingleBXDF(const std::string& name, std::shared_ptr<BXDF> bxdf, const std::shared_ptr<Texture<vec3>>& color) :
	Material(name), m_bxdf(bxdf), m_color(color)
{

}

std::shared_ptr<BSDF> MaterialSingleBXDF::get_bsdf(const IntersectionInfo& hitInfo) const
{
	std::shared_ptr<BSDF> bsdf = std::make_shared<BSDF>(hitInfo.shadingNormal);

	bsdf->add_bxdf(
		m_bxdf,
		m_color->evaluate(hitInfo)
	);

	return bsdf;
}

}; //namespace fr