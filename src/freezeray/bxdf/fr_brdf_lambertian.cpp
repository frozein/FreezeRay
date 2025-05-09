#include "freezeray/bxdf/fr_brdf_lambertian.hpp"
#include "freezeray/fr_globals.hpp"

//-------------------------------------------//

namespace fr
{

BRDFLambertian::BRDFLambertian() :
	BXDF(BXDFflags::REFLECTION)
{

}

vec3 BRDFLambertian::f(const vec3& wi, const vec3& wo) const
{
	return same_hemisphere(wi, wo) ? FR_INV_PI : 0.0f;
}

vec3 BRDFLambertian::sample_f(vec3& wi, const vec3& wo, const vec2& u, float& pdfVal) const
{
	//generate random vector in positive hemisphere:
	//---------------
	float r = std::sqrtf(u.x);
	float theta = FR_2_PI * u.y;

	vec2 xz = vec2(r * std::cos(theta), r * std::sin(theta));
	float y = std::sqrtf(1.0f - xz.x * xz.x - xz.y * xz.y);
	wi = vec3(xz.x, wo.y < 0.0f ? -y : y, xz.y);

	//return:
	//---------------
	pdfVal = pdf(wi, wo);
	return f(wi, wo);
}

float BRDFLambertian::pdf(const vec3& wi, const vec3& wo) const
{
	return same_hemisphere(wi, wo) ? std::abs(cos_theta(wi)) * FR_INV_PI : 0.0f;
}

}; //namespace fr