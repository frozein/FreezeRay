#include "freezeray/material/fr_material_single_bxdf.hpp"

//-------------------------------------------//

namespace fr
{

MaterialSingleBXDF::MaterialSingleBXDF(const std::string& name, std::shared_ptr<BXDF> bxdf, const std::shared_ptr<Texture<vec3>>& color) :
	Material(name, bxdf->is_delta(), bxdf->type()), m_bxdf(bxdf), m_color(color)
{

}

vec3 MaterialSingleBXDF::bsdf_f(const IntersectionInfo& hitInfo, const vec3& wiWorld, const vec3& woWorld) const
{
	vec3 wi, wo;
	world_to_local(hitInfo.worldNormal, wiWorld, woWorld, wi, wo);

	return m_color->evaluate(hitInfo) * m_bxdf->f(wi, wo);
}

vec3 MaterialSingleBXDF::bsdf_sample_f(const IntersectionInfo& hitInfo, vec3& wiWorld, const vec3& woWorld, const vec3& u, float& pdf) const
{
	vec3 wi;
	vec3 wo = world_to_local(hitInfo.worldNormal, woWorld);

	vec3 f = m_color->evaluate(hitInfo) * m_bxdf->sample_f(wi, wo, u.xy(), pdf);

	wiWorld = local_to_world(hitInfo.worldNormal, wi);
	return f;
}

float MaterialSingleBXDF::bsdf_pdf(const IntersectionInfo& hitInfo, const vec3& wiWorld, const vec3& woWorld) const
{
	vec3 wi, wo;
	world_to_local(hitInfo.worldNormal, wiWorld, woWorld, wi, wo);

	return m_bxdf->pdf(wi, wo);
}

}; //namespace fr