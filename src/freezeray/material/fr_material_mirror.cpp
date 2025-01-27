#include "freezeray/material/fr_material_mirror.hpp"

//-------------------------------------------//

namespace fr
{

MaterialMirror::MaterialMirror(const std::string& name, const std::shared_ptr<Texture<vec3>>& color) : 
	Material(name, true, BXDFType::REFLECTION), m_color(color), m_fresnel(vec3(1.0f)), 
	m_brdf(std::make_shared<FresnelConstant>(m_fresnel))
{

}

vec3 MaterialMirror::bsdf_f(const IntersectionInfo& hitInfo, const vec3& wiWorld, const vec3& woWorld) const
{
	//delta distribution
	return vec3(0.0f);
}

vec3 MaterialMirror::bsdf_sample_f(const IntersectionInfo& hitInfo, vec3& wiWorld, const vec3& woWorld, const vec3& u, float& pdf) const
{
	vec3 wi;
	vec3 wo = world_to_local(hitInfo.worldNormal, woWorld);

	vec3 f = m_color->evaluate(hitInfo) * m_brdf.sample_f(wi, wo, u.xy(), pdf);

	wiWorld = local_to_world(hitInfo.worldNormal, wi);
	return f;
}

float MaterialMirror::bsdf_pdf(const IntersectionInfo& hitInfo, const vec3& wiWorld, const vec3& woWorld) const
{
	//delta distribution
	return 1.0f;
}

}; //namespace fr