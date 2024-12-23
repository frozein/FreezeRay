#include "rurt/material/material_single_bxdf.hpp"

//-------------------------------------------//

namespace rurt
{

MaterialSingleBXDF::MaterialSingleBXDF(const std::string& name, std::shared_ptr<const BXDF> bxdf, const vec3& color) :
	Material(name, bxdf->is_delta(), bxdf->type()), m_bxdf(bxdf), m_color(color)
{

}

vec3 MaterialSingleBXDF::bsdf_f(const IntersectionInfo& hitInfo, const vec3& wiWorld, const vec3& woWorld) const
{
	vec3 wi, wo;
	world_to_local(hitInfo.worldNormal, wiWorld, woWorld, wi, wo);

	return m_color * m_bxdf->f(wi, wo);
}

vec3 MaterialSingleBXDF::bsdf_sample_f(const IntersectionInfo& hitInfo, vec3& wiWorld, const vec3& woWorld, const vec2& u, float& pdf) const
{
	vec3 wi;
	vec3 wo = world_to_local(hitInfo.worldNormal, woWorld);

	vec3 f = m_color * m_bxdf->sample_f(wi, wo, pdf);

	wiWorld = local_to_world(hitInfo.worldNormal, wi);
	return f;
}

float MaterialSingleBXDF::bsdf_pdf(const IntersectionInfo& hitInfo, const vec3& wiWorld, const vec3& woWorld) const
{
	vec3 wi, wo;
	world_to_local(hitInfo.worldNormal, wiWorld, woWorld, wi, wo);

	return m_bxdf->pdf(wi, wo);
}

}; //namespace rurt