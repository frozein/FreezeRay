#include "rurt/bxdf/btdf_specular.hpp"
#include "rurt/globals.hpp"

//-------------------------------------------//

namespace rurt
{

BTDFSpecular::BTDFSpecular(const vec3& color, float etaI, float etaT, std::shared_ptr<const Fresnel> fresnel) :
	BXDF(BXDFType::TRANSMISSION), m_color(srgb_to_linear(color)), m_etaI(etaI), m_etaT(etaT), m_fresnel(fresnel)
{

}

vec3 BTDFSpecular::f(const HitInfo& info, const vec3& wi, const vec3& wo) const
{
	//delta distribution, f = 0 except for 1 point
	return vec3(0.0f);
}

vec3 BTDFSpecular::sample_f(const HitInfo& info, vec3& wi, const vec3& wo, float& pdfVal) const
{
	//get indices of refraction:
	//---------------
	bool entering = cos_theta(wo) > 0.0f;

	float etaI;
	float etaT;
	if(entering)
	{
		etaI = m_etaI;
		etaT = m_etaT;
	}
	else //not entering, need to swap
	{
		etaI = m_etaT;
		etaT = m_etaI;
	}

	//refract:
	//---------------
	float eta = etaI / etaT;

    float cosThetaI = std::abs(cos_theta(wo));
    float sinTheta2I = std::max(0.0f, 1.0f - cosThetaI * cosThetaI);
    float sinTheta2T = eta * eta * sinTheta2I;

    if(sinTheta2T >= 1.0f)
	{
		pdfVal = 1.0f;
		return vec3(0.0f);
	}
	
    float cosThetaT = std::sqrt(1.0f - sinTheta2T);
    wi = eta * -1.0f * wo + (eta * cosThetaI - cosThetaT) * (entering ? RURT_UP_DIR : RURT_DOWN_DIR);
	
	//return:
	//---------------
	float cosTheta = cos_theta(wi);

	pdfVal = 1.0f;
	return m_color * (1.0f - m_fresnel->evaluate(cosTheta)) / std::abs(cosTheta);
}

float BTDFSpecular::pdf(const HitInfo& info, const vec3& wi, const vec3& wo) const
{
	//delta distribution, pdf = 0 except for 1 point
	return 0.0f;
}

}; //namespace rurt