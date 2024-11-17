#include "rurt/brdf/brdf_specular_reflection.hpp"
#include "rurt/globals.hpp"

//-------------------------------------------//

namespace rurt
{

BRDFSpecularReflection::BRDFSpecularReflection(const vec3& color) :
	m_color(srgb_to_linear(color))
{

}

vec3 BRDFSpecularReflection::f(const HitInfo& info, const vec3& wi, const vec3& wo) const
{
	//delta distribution, f = 0 except for 1 point
	return vec3(0.0f);
}

vec3 BRDFSpecularReflection::sample_f(const HitInfo& info, vec3& wi, const vec3& wo, float& pdfVal) const
{
	//compute specular reflection direction:
	//---------------
	wi = vec3(-wo.x, wo.y, -wo.z);

	//return:
	//---------------
	pdfVal = 1.0f;
	return m_color / std::abs(cos_theta(wi));
}

float BRDFSpecularReflection::pdf(const HitInfo& info, const vec3& wi, const vec3& wo) const
{
	//delta distribution, pdf = 0 except for 1 point
	return 0.0f;
}

}; //namespace rurt