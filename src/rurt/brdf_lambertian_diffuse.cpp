#include "brdf_lambertian_diffuse.hpp"
#include "constants.hpp"

//-------------------------------------------//

namespace rurt
{

BRDFLambertianDiffuse::BRDFLambertianDiffuse(vec3 color) : m_color(color)
{

}

vec3 BRDFLambertianDiffuse::f(const HitInfo& info, const vec3& i, const vec3& o, float& pdfVal) const
{
	pdfVal = pdf(info, i, o);
	return m_color * RURT_INV_PI;
}

float BRDFLambertianDiffuse::pdf(const HitInfo& info, const vec3& i, const vec3& o) const
{
	return RURT_INV_2_PI;
}

const vec3& BRDFLambertianDiffuse::get_color() const
{
	return m_color;
}

}; //namespace rurt