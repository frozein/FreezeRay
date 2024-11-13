#include "rurt/brdf/brdf_lambertian_diffuse.hpp"
#include "rurt/constants.hpp"

//-------------------------------------------//

namespace rurt
{

BRDFLambertianDiffuse::BRDFLambertianDiffuse(const vec3& color)
{
	m_color.r = std::powf(color.r, RURT_GAMMA);
	m_color.g = std::powf(color.g, RURT_GAMMA);
	m_color.b = std::powf(color.b, RURT_GAMMA);
}

vec3 BRDFLambertianDiffuse::f(const HitInfo& info, const vec3& i, const vec3& o, float& pdfVal) const
{
	pdfVal = pdf(info, i, o);
	return m_color * RURT_INV_PI;;
}

float BRDFLambertianDiffuse::pdf(const HitInfo& info, const vec3& i, const vec3& o) const
{
	return RURT_INV_2_PI;
}

}; //namespace rurt