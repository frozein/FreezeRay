#include "rurt/bxdf/brdf_specular.hpp"
#include "rurt/globals.hpp"

//-------------------------------------------//

namespace rurt
{

BRDFSpecular::BRDFSpecular(std::shared_ptr<const Fresnel> fresnel) :
	BXDF(true, BXDFType::REFLECTION), m_fresnel(fresnel)
{

}

vec3 BRDFSpecular::f(const vec3& wi, const vec3& wo) const
{
	//delta distribution, f = 0 except for 1 point
	return vec3(0.0f);
}

vec3 BRDFSpecular::sample_f(vec3& wi, const vec3& wo, const vec2& u, float& pdfVal) const
{
	//compute specular reflection direction:
	//---------------
	wi = vec3(-wo.x, wo.y, -wo.z);

	//return:
	//---------------
	float cosTheta = cos_theta(wi);

	pdfVal = 1.0f;
	return m_fresnel->evaluate(cosTheta) / std::abs(cosTheta);
}

float BRDFSpecular::pdf(const vec3& wi, const vec3& wo) const
{
	//delta distribution, pdf = 0 except for 1 point
	return 0.0f;
}

}; //namespace rurt