#include "rurt/bxdf/brdf_specular.hpp"
#include "rurt/globals.hpp"

//-------------------------------------------//

namespace rurt
{

BRDFSpecular::BRDFSpecular(const vec3& color, std::shared_ptr<const Fresnel> fresnel) :
	BXDF(BXDFType::REFLECTION), m_color(srgb_to_linear(color)), m_fresnel(fresnel)
{

}

vec3 BRDFSpecular::f(const HitInfo& info, const vec3& wi, const vec3& wo) const
{
	//delta distribution, f = 0 except for 1 point
	return vec3(0.0f);
}

vec3 BRDFSpecular::sample_f(const HitInfo& info, vec3& wi, const vec3& wo, float& pdfVal) const
{
	//compute specular reflection direction:
	//---------------
	wi = vec3(-wo.x, wo.y, -wo.z);

	//return:
	//---------------
	float cosTheta = cos_theta(wi);

	pdfVal = 1.0f;
	return m_color * m_fresnel->evaluate(cosTheta) / std::abs(cosTheta);
}

float BRDFSpecular::pdf(const HitInfo& info, const vec3& wi, const vec3& wo) const
{
	//delta distribution, pdf = 0 except for 1 point
	return 0.0f;
}

}; //namespace rurt