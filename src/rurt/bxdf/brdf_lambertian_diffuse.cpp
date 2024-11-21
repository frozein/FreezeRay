#include "rurt/bxdf/brdf_lambertian_diffuse.hpp"
#include "rurt/globals.hpp"

//-------------------------------------------//

namespace rurt
{

BRDFLambertianDiffuse::BRDFLambertianDiffuse() :
	BXDF(false, BXDFType::REFLECTION)
{

}

vec3 BRDFLambertianDiffuse::f(const vec3& wi, const vec3& wo) const
{
	return RURT_INV_PI;
}

vec3 BRDFLambertianDiffuse::sample_f(vec3& wi, const vec3& wo, float& pdfVal) const
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
	pdfVal = pdf(wi, wo);
	return f(wi, wo);
}

float BRDFLambertianDiffuse::pdf(const vec3& wi, const vec3& wo) const
{
	return RURT_INV_2_PI;
}

}; //namespace rurt