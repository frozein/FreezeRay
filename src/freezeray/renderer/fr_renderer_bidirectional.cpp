#include "freezeray/renderer/fr_renderer_bidirectional.hpp"
#include "freezeray/fr_globals.hpp"

//-------------------------------------------//

namespace fr
{

//-------------------------------------------//

RendererBidirectional::RendererBidirectional(std::shared_ptr<const Camera> cam, uint32_t imageW, uint32_t imageH, uint32_t maxDepth, uint32_t samplesPerPixel, bool importanceSampling, bool multipleImportanceSampling) :
	Renderer(cam, imageW, imageH),
	m_maxDepth(maxDepth), 
	m_samplesPerPixel(samplesPerPixel), 
	m_importanceSampling(importanceSampling),
	m_mis(multipleImportanceSampling)
{

}

RendererBidirectional::~RendererBidirectional()
{

}

vec3 RendererBidirectional::li(const std::shared_ptr<PRNG>& prng, const std::shared_ptr<const Scene>& scene, const Ray& ray) const
{
	vec3 li = vec3(0.0f);
	for(uint32_t i = 0; i < m_samplesPerPixel; i++)
		li = li + trace_bidirectional_path(prng, scene, ray);

	return li / (float)m_samplesPerPixel;
}

vec3 RendererBidirectional::trace_bidirectional_path(const std::shared_ptr<PRNG>& prng, const std::shared_ptr<const Scene>& scene, const Ray& ray) const
{
	//generate camera subpath:
	//---------------	
	std::vector<PathVertex> cameraSubpath;

	//TODO: treat the camera properly
	float cameraPdfPos = 1.0f;
	float cameraPdfDir = 1.0f;
	vec3 cameraMult = vec3(1.0f);
	
	PathVertex cameraStart = PathVertex::from_camera(m_cam, ray, cameraMult);
	cameraSubpath.push_back(cameraStart);

	trace_walk(prng, scene, ray, cameraMult, cameraPdfDir, cameraSubpath);

	//generate light subpath:
	//---------------
	std::vector<PathVertex> lightSubpath;

	uint32_t numLights = (uint32_t)scene->get_lights().size();
	if(numLights == 0)
		return vec3(0.0f);

	uint32_t lightIdx = prng->randi() % numLights;
	const std::shared_ptr<const Light>& light = scene->get_lights()[lightIdx];
	float lightPdf = 1.0f / numLights;

	vec3 u1 = prng->rand3f();
	vec3 u2 = prng->rand3f();

	Ray lightRay;
	vec3 lightNormal;
	float lightPdfPos;
	float lightPdfDir;
	vec3 le = light->sample_le(u1, u2, lightRay, lightNormal, lightPdfPos, lightPdfDir);
	if(lightPdfPos > 0.0f && lightPdfDir > 0.0f)
	{
		IntersectionInfo lightHit;
		lightHit.pos = lightRay.origin();
		lightHit.shadingNormal = lightNormal;

		vec3 lightMult = le * std::abs(dot(lightNormal, lightRay.direction())) / (lightPdf * lightPdfDir * lightPdfPos);

		PathVertex lightStart = PathVertex::from_light(light, lightHit, lightMult);
		lightSubpath.push_back(lightStart);

		trace_walk(prng, scene, lightRay, lightMult, lightPdfDir, lightSubpath);
	}

	//connect subpaths:
	//---------------
	std::vector<std::pair<vec3, uint32_t>> contribs;
	contribs.resize(m_maxDepth + 1);

	for(uint32_t t = 2; t <= cameraSubpath.size(); t++) //TODO: allow t=1
	for(uint32_t s = 0; s <= lightSubpath .size(); s++)
	{
		uint32_t depth = t + s - 1;
		if(depth <= 0 || depth > m_maxDepth)
			continue;

		vec3 contrib = vec3(0.0f);

		if(s == 0) //only use camera path, just look at last vertex
		{
			const PathVertex end = cameraSubpath[t - 1];
			if(end.type != PathVertex::Type::LIGHT && !end.intersection.light)
				contrib = vec3(0.0f);
			else
			{
				if(end.intersection.light)
					contrib = end.mult * end.intersection.light->le(end.intersection, end.intersection.wo);
				else
				{
					const std::vector<std::shared_ptr<const Light>>& infiniteLights = scene->get_infinite_lights();
					for(uint32_t i = 0; i < infiniteLights.size(); i++)
						contrib = contrib + end.mult * infiniteLights[i]->le(end.intersection, end.intersection.wo);
				}
			}
		}
		else if(t == 1)
		{
			//TODO!!!
		}
		else if(s == 1) //only 1 vertex from light path, just do a standard direct light sample
		{
			const PathVertex end = cameraSubpath[t - 1];
			if(!end.intersection.bsdf || end.intersection.bsdf->is_delta())
				contrib = vec3(0.0f);
			else
				contrib = end.mult * sample_one_light(prng, scene, end.intersection, end.intersection.wo);
		}
		else //arbitrary connection
		{
			const PathVertex endCam = cameraSubpath[t - 1];
			const PathVertex endLight = lightSubpath[s - 1];
			if(!endCam  .intersection.bsdf || endCam  .intersection.bsdf->is_delta() ||
			   !endLight.intersection.bsdf || endLight.intersection.bsdf->is_delta())
				contrib = vec3(0.0f);
			else
			{
				vec3 wiCam   = normalize(endLight.intersection.pos - endCam  .intersection.pos);
				vec3 wiLight = normalize(endCam  .intersection.pos - endLight.intersection.pos);

				contrib = endCam  .mult * endCam  .intersection.bsdf->f(wiCam  , endCam  .intersection.wo, BXDFflags::ALL) *
				          endLight.mult * endLight.intersection.bsdf->f(wiLight, endLight.intersection.wo, BXDFflags::ALL);

				if(contrib != vec3(0.0f))
				{
					VisibilityTestInfo visTest;
					visTest.endPos = endLight.intersection.pos;
					visTest.infinite = false;

					vec3 toLight = endLight.intersection.pos - endCam.intersection.pos;
					bool visible = trace_visibility_ray(scene, endCam.intersection, toLight, visTest);

					vec3 to = endCam.intersection.pos - endLight.intersection.pos;
					float g = 1.0f / dot(to, to);
					to = to * std::sqrtf(g);

					g *= std::abs(dot(to, endCam.intersection.shadingNormal));
					g *= std::abs(dot(to, endLight.intersection.shadingNormal));

					contrib = contrib * g * (float)visible;
				}
			}
		}

		contribs[depth].first = contribs[depth].first + contrib;
		contribs[depth].second++;
	}

	//divide contributions by total number of feasible strategies (uniform weighting):
	//---------------
	vec3 l = vec3(0.0f);
	for(uint32_t i = 0; i < contribs.size(); i++)
	{
		if(contribs[i].second == 0)
			continue;
		
		l = l + contribs[i].first / (float)contribs[i].second;
	}

	return l;
}

void RendererBidirectional::trace_walk(const std::shared_ptr<PRNG>& prng, const std::shared_ptr<const Scene>& scene, const Ray& ray, vec3 mult, float pdf, std::vector<PathVertex>& vertices) const
{
	Ray curRay = ray;
	for(uint32_t i = 0; i < m_maxDepth; i++)
	{
		//compute intersection
		IntersectionInfo hitInfo;
		bool hit = scene->intersect(curRay, hitInfo);

		//negate ray direction to get wo:
		vec3 wo = -1.0f * curRay.direction();

		//add infinite lights + break if nothing hit
		if(!hit)
		{
			PathVertex lightVert = PathVertex::from_light(nullptr, hitInfo, mult);
			vertices.push_back(lightVert);

			break;
		}

		//add vertex
		PathVertex vert = PathVertex::from_surface(hitInfo, mult);
		vertices.push_back(vert);

		//evaluate bsdf:
		BXDFflags bsdfFlags = hitInfo.bsdf->get_flags();
		
		vec3 f;
		float pdf;
		vec3 wi;
		BXDFflags sampledFlags;

		if((bsdfFlags & BXDFflags::DELTA) != BXDFflags::NONE || m_importanceSampling)
		{
			vec3 u = prng->rand3f();
			f = hitInfo.bsdf->sample_f(wi, wo, u, pdf, BXDFflags::ALL, sampledFlags);
		}
		else
		{
			if(((bsdfFlags & BXDFflags::REFLECTION)   != BXDFflags::NONE) &&
			   ((bsdfFlags & BXDFflags::TRANSMISSION) != BXDFflags::NONE))
			{
				wi = prng->rand_sphere();
				pdf = FR_INV_PI;
			}
			else if((bsdfFlags & BXDFflags::REFLECTION) != BXDFflags::NONE)
			{
				wi = prng->rand_hemisphere(hitInfo.shadingNormal);
				pdf = FR_INV_2_PI;
			}
			else if((bsdfFlags & BXDFflags::TRANSMISSION) != BXDFflags::NONE)
			{
				wi = prng->rand_hemisphere(-1.0f * hitInfo.shadingNormal);
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
		float cosTheta = std::abs(dot(wi, hitInfo.shadingNormal));
		mult = mult * (f * cosTheta / pdf);

		//TODO: account for asymmetry

		//set new ray
		vec3 bounceDir = wi;
		vec3 bouncePos = hitInfo.pos + FR_EPSILON * normalize(wi);

		curRay = Ray(bouncePos, bounceDir);

		//TODO: russian roulette?
	}
}

//-------------------------------------------//

RendererBidirectional::PathVertex RendererBidirectional::PathVertex::from_surface(const IntersectionInfo& surface, const vec3& mult)
{
	PathVertex vert;
	vert.type = Type::SURFACE;
	vert.intersection = surface;
	vert.mult = mult;

	return vert;
}

RendererBidirectional::PathVertex RendererBidirectional::PathVertex::from_light(std::shared_ptr<const Light> light, const IntersectionInfo& hitInfo, const vec3& mult)
{
	PathVertex vert;
	vert.type = Type::LIGHT;
	vert.intersection = hitInfo;
	vert.intersection.light = light;
	vert.mult = mult;

	return vert;
}

RendererBidirectional::PathVertex RendererBidirectional::PathVertex::from_camera(std::shared_ptr<const Camera> camera, const Ray& ray, const vec3& mult)
{
	PathVertex vert;
	vert.type = Type::CAMERA;
	vert.intersection.camera = camera;
	vert.intersection.pos = ray.origin();
	vert.intersection.shadingNormal = ray.direction();
	vert.mult = mult;

	return vert;
}

}; //namespace fr