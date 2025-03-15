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

vec3 RendererPath::li(const std::shared_ptr<const Scene>& scene, const Ray& ray) const
{
	vec3 li = vec3(0.0f);
	for(uint32_t i = 0; i < m_samplesPerPixel; i++)
		li = li + trace_path(scene, ray);

	return li / (float)m_samplesPerPixel;
}

vec3 RendererPath::trace_path(const std::shared_ptr<const Scene>& scene, const Ray& ray) const
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

		//get bsdf flags
		BXDFflags bsdfFlags = hitInfo.bsdf->get_flags();
		
		//add contribution from light sources
		if((bsdfFlags & BXDFflags::DELTA) == BXDFflags::NONE)
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
		BXDFflags sampledFlags;

		if((bsdfFlags & BXDFflags::DELTA) != BXDFflags::NONE || m_importanceSampling)
		{
			vec3 u = vec3((float)rand() / RAND_MAX, (float)rand() / RAND_MAX, (float)rand() / RAND_MAX);
			f = hitInfo.bsdf->sample_f(wi, wo, u, pdf, BXDFflags::ALL, sampledFlags);
		}
		else
		{
			if(((bsdfFlags & BXDFflags::REFLECTION)   != BXDFflags::NONE) &&
			   ((bsdfFlags & BXDFflags::TRANSMISSION) != BXDFflags::NONE))
			{
				wi = random_dir_sphere();
				pdf = FR_INV_PI;
			}
			else if((bsdfFlags & BXDFflags::REFLECTION) != BXDFflags::NONE)
			{
				wi = random_dir_hemisphere(hitInfo.worldNormal);
				pdf = FR_INV_2_PI;
			}
			else if((bsdfFlags & BXDFflags::TRANSMISSION) != BXDFflags::NONE)
			{
				wi = random_dir_hemisphere(-1.0f * hitInfo.worldNormal);
				pdf = FR_INV_2_PI;
			}
			else
			{
				//shouldnt ever reach
				
				wi = 0.0f;
				pdf = 0.0f;
			}

			f = hitInfo.bsdf->f(wi, wo, BXDFflags::ALL);
			sampledFlags = bsdfFlags;
		}

		//break if 0 BRDF or PDF
		if(f == 0.0f || pdf == 0.0f)
			break;

		//apply brdf to current color
		float cosTheta = std::abs(dot(wi, hitInfo.worldNormal));
		mult = mult * (f * cosTheta / pdf);

		//set new ray
		vec3 bounceDir = wi;
		vec3 bouncePos = hitInfo.worldPos + FR_EPSILON * normalize(wi);

		curRay = Ray(bouncePos, bounceDir);
		deltaBounce = (sampledFlags & BXDFflags::DELTA) != BXDFflags::NONE;

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