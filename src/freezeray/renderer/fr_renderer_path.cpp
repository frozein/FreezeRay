#include "freezeray/renderer/fr_renderer_path.hpp"
#include "freezeray/fr_globals.hpp"

//-------------------------------------------//

namespace fr
{

RendererPath::RendererPath(std::shared_ptr<const Camera> cam, uint32_t imageW, uint32_t imageH, uint32_t maxDepth, uint32_t samplesPerPixel, bool importanceSampling, bool multipleImportanceSampling) :
	Renderer(cam, imageW, imageH),
	m_maxDepth(maxDepth), 
	m_samplesPerPixel(samplesPerPixel), 
	m_importanceSampling(importanceSampling),
	m_mis(multipleImportanceSampling)
{

}

RendererPath::~RendererPath()
{

}

vec3 RendererPath::li(const std::shared_ptr<const Scene>& scene, const Ray& ray)
{
	vec3 li = vec3(0.0f);
	for(uint32_t i = 0; i < m_samplesPerPixel; i++)
		li = li + trace_path(scene, ray);

	return li / (float)m_samplesPerPixel;
}

vec3 RendererPath::trace_path(const std::shared_ptr<const Scene>& scene, const Ray& ray)
{
	vec3 light = vec3(0.0f);
	vec3 mult = vec3(1.0f);

	bool deltaBounce = false;

	Ray curRay = ray;
	for(uint32_t i = 0; i < m_maxDepth; i++)
	{
		IntersectionInfo hitInfo;
		bool hit = scene->intersect(curRay, hitInfo);

		//negate ray direction to get wo:
		vec3 wo = -1.0f * curRay.direction();

		//add emitted light if first bounce or delta bounce
		if(i == 0 || deltaBounce)
		{
			if(hit)
			{
				if(hitInfo.light != nullptr)
					light = light + mult * hitInfo.light->le(hitInfo, wo);
			}
			else
			{
				const std::vector<std::shared_ptr<const Light>>& infiniteLights = scene->get_infinite_lights();
				for(uint32_t i = 0; i < infiniteLights.size(); i++)
					light = light + mult * infiniteLights[i]->le(hitInfo, wo);
			}
		}

		//break if nothing hit
		if(!hit)
			break;

		//add contribution from light sources
		if(!hitInfo.material->bsdf_is_delta())
		{
			if(m_mis)
				light = light + mult * sample_one_light_mis(scene, hitInfo, wo);
			else
				light = light + mult * sample_one_light(scene, hitInfo, wo);
		}

		//evaluate bsdf:
		vec3 f;
		float pdf;
		vec3 wi;

		if(hitInfo.material->bsdf_is_delta() || m_importanceSampling)
		{
			vec3 u = vec3((float)rand() / RAND_MAX, (float)rand() / RAND_MAX, (float)rand() / RAND_MAX);
			f = hitInfo.material->bsdf_sample_f(hitInfo, wi, wo, u, pdf);
		}
		else
		{
			switch(hitInfo.material->bsdf_type())
			{
			case BXDFType::REFLECTION:
				wi = random_dir_hemisphere(hitInfo.worldNormal);
				pdf = FR_INV_2_PI;
				break;
			case BXDFType::TRANSMISSION:
				wi = random_dir_hemisphere(-1.0f * hitInfo.worldNormal);
				pdf = FR_INV_2_PI;
				break;
			case BXDFType::BOTH:
			default:
				wi = random_dir_sphere();
				pdf = 2.0f * FR_INV_2_PI;
				break;
			}

			f = hitInfo.material->bsdf_f(hitInfo, wi, wo);
		}

		//break if 0 BRDF or PDF
		if(f == vec3(0.0f) || pdf == 0.0f)
			break;

		//apply brdf to current color
		float cosTheta = std::abs(dot(wi, hitInfo.worldNormal));
		mult = mult * (f * cosTheta / pdf);

		//set new ray
		vec3 bounceDir = wi;
		vec3 bouncePos = hitInfo.worldPos;
		
		if(dot(bounceDir, hitInfo.worldNormal) > 0.0f) //reflection
			bouncePos = bouncePos + FR_EPSILON * hitInfo.worldNormal;
		else //transmission
		{
			bool entering = dot(wo, hitInfo.worldNormal) > 0.0f;
			bouncePos = bouncePos + (entering ? -FR_EPSILON : FR_EPSILON) * hitInfo.worldNormal;
		}

		curRay = Ray(bouncePos, bounceDir);
		deltaBounce = hitInfo.material->bsdf_is_delta();

		//russian roulette to exit based on color:
		float maxComp = std::max(std::max(mult.r, mult.g), mult.b);
		float q = std::max(0.05f, 1.0f - maxComp);
		
		float roulette = (float)rand() / RAND_MAX;
		if(roulette < q)
			break;
		else
			mult = mult / (1.0f - q);
	}

	return light;
}

}; //namespace fr