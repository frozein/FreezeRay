#include "rurt/bxdf/brdf_lambertian_diffuse.hpp"
#include "rurt/globals.hpp"

//-------------------------------------------//

namespace rurt
{

BRDFLambertianDiffuse::BRDFLambertianDiffuse(const vec3& color) :
	BXDF(BXDFType::REFLECTION), m_color(srgb_to_linear(color))
{

}

vec3 BRDFLambertianDiffuse::f(const HitInfo& info, const vec3& wi, const vec3& wo) const
{
	return m_color * RURT_INV_PI;
}

vec3 BRDFLambertianDiffuse::sample_f(const HitInfo& info, vec3& wi, const vec3& wo, float& pdfVal) const
{
	//generate random vector in positive hemisphere:
	//---------------
	while(true)
	{
		wi.x = ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f;
		wi.y = ((float)rand() / (float)RAND_MAX);
		wi.z = ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f;

		float lenSqr = dot(wi, wi);
		if(lenSqr > RURT_EPSILON && lenSqr <= 1.0f)
		{
			wi = wi / std::sqrtf(lenSqr);
			break;
		}
	}

	//return:
	//---------------
	pdfVal = pdf(info, wi, wo);
	return f(info, wi, wo);
}

float BRDFLambertianDiffuse::pdf(const HitInfo& info, const vec3& wi, const vec3& wo) const
{
	return RURT_INV_2_PI;
}

}; //namespace rurt