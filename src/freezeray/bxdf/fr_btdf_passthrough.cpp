#include "freezeray/bxdf/fr_btdf_passthrough.hpp"

//-------------------------------------------//

namespace fr
{

BTDFPassthrough::BTDFPassthrough() :
	BXDF(BXDFflags::TRANSMISSION | BXDFflags::DELTA)
{

}

vec3 BTDFPassthrough::f(const vec3& wi, const vec3& wo) const
{
	//delta distribution, f = 0 except for 1 point
	return vec3(0.0f);
}

vec3 BTDFPassthrough::sample_f(vec3& wi, const vec3& wo, const vec2& u, float& pdfVal) const
{
	wi = -1.0f * wo;
	
	pdfVal = 1.0f;
	return 1.0f;
}

float BTDFPassthrough::pdf(const vec3& wi, const vec3& wo) const
{
	//delta distribution, pdf = 0 except for 1 point
	return 0.0f;
}

}; //namespace fr