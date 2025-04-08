#include "freezeray/renderer/fr_renderer_bidirectional.hpp"
#include "freezeray/fr_globals.hpp"

//-------------------------------------------//

namespace fr
{

//-------------------------------------------//

template<typename T>
class ScopedAssignment
{
public:
	ScopedAssignment(T* dst = nullptr, T val = T()) : m_dst(dst)
	{
		if(dst)
		{
			m_prevVal = *dst;
			*dst = val;
		}
	}

	~ScopedAssignment()
	{
		if(m_dst)
			*m_dst = m_prevVal;
	}

    ScopedAssignment &operator=(ScopedAssignment &&other) 
	{
        if(m_dst) 
			*m_dst = m_prevVal;
		
        m_dst = other.m_dst;
        m_prevVal = other.m_prevVal;
        other.m_dst = nullptr;

        return *this;
    }

private:
	T* m_dst;
	T m_prevVal;
};

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

		PathVertex lightStart = PathVertex::from_light(light, lightHit, lightMult, lightPdfPos * lightPdf);
		lightSubpath.push_back(lightStart);

		trace_walk(prng, scene, lightRay, lightMult, lightPdfDir, lightSubpath);
	}

	//connect subpaths:
	//---------------
	vec3 l = vec3(0.0f);

	for(uint32_t t = 2; t <= cameraSubpath.size(); t++) //TODO: allow t=1
	for(uint32_t s = 0; s <= lightSubpath .size(); s++)
	{
		uint32_t depth = t + s - 1;
		if(depth <= 0 || depth > m_maxDepth)
			continue;

		PathVertex sampled;
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
			{
				uint32_t numLights = (uint32_t)scene->get_lights().size();
				uint32_t lightIdx = prng->randi() % numLights;
			
				vec3 u = prng->rand3f();
				vec3 wi;
				VisibilityTestInfo visInfo;
				float pdf;
			
				const std::shared_ptr<const Light>& light = scene->get_lights()[lightIdx];
				vec3 li = light->sample_li(end.intersection, u, wi, visInfo, pdf);
			
				pdf /= (float)numLights;

				if(pdf == 0.0f)
					contrib = vec3(0.0f);
				else
				{
					IntersectionInfo sampledHit;
					sampledHit.pos = visInfo.endPos;

					sampled = PathVertex::from_light(light, sampledHit, li / pdf, 0.0f);
					sampled.pdfFwd = sampled.pdf_light_origin(scene, end);
				
					contrib = sampled.mult * end.mult * end.f(sampled) * std::abs(dot(wi, end.intersection.shadingNormal));

					if(contrib == vec3(0.0f) || !trace_visibility_ray(scene, end.intersection, wi, visInfo))
						contrib = vec3(0.0f);
				}
			}
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

		if(contrib != vec3(0.0f))
		{
			if(m_mis)
				l = l + contrib * mis_weight(scene, cameraSubpath, lightSubpath, sampled, s, t);
			else
				l = l + contrib * uniform_weight(cameraSubpath, lightSubpath, s, t);
		}
	}

	return l;
}

void RendererBidirectional::trace_walk(const std::shared_ptr<PRNG>& prng, const std::shared_ptr<const Scene>& scene, const Ray& ray, vec3 mult, float pdf, std::vector<PathVertex>& vertices) const
{
	float pdfFwd = pdf;
	float pdfRev = 0.0f;

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
			PathVertex lightVert = PathVertex::from_light(nullptr, hitInfo, mult, pdfFwd);
			vertices.push_back(lightVert);

			break;
		}

		//add vertex
		PathVertex vert = PathVertex::from_surface(hitInfo, mult, pdfFwd, vertices[vertices.size() - 1]);
		vertices.push_back(vert);

		PathVertex& prevVert = vertices[vertices.size() - 2];
		PathVertex& curVert = vertices[vertices.size() - 1];

		//evaluate bsdf:
		BXDFflags bsdfFlags = hitInfo.bsdf->get_flags();
		
		vec3 f;
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
		
		//update pdfs
		pdfFwd = pdf;
		pdfRev = hitInfo.bsdf->pdf(wo, wi, BXDFflags::ALL);
		if((sampledFlags & BXDFflags::DELTA) != BXDFflags::NONE)
		{
			pdfFwd = 0.0f;
			pdfRev = 0.0f;
			curVert.delta = true;
		}

		//set new ray
		vec3 bounceDir = wi;
		vec3 bouncePos = hitInfo.pos + FR_EPSILON * normalize(wi);

		curRay = Ray(bouncePos, bounceDir);

		//compute reverse area density at previous vertex
		prevVert.pdfRev = curVert.convert_density(pdfRev, prevVert);

		//TODO: russian roulette?
	}
}

float RendererBidirectional::mis_weight(const std::shared_ptr<const Scene>& scene, std::vector<PathVertex>& cameraSubpath, std::vector<PathVertex>& lightSubpath,
                                        PathVertex& sampled, uint32_t s, uint32_t t) const
{
	//early return for paths of depth 2:
	//---------------
	if(s + t == 2)
		return 1.0f;

	//modify pdfs for connected vertices:
	//---------------
	PathVertex* lightEnd      = s > 0 ? &lightSubpath [s - 1] : nullptr;
	PathVertex* cameraEnd     = t > 0 ? &cameraSubpath[t - 1] : nullptr;
	PathVertex* lightEndPrev  = s > 1 ? &lightSubpath [s - 2] : nullptr;
	PathVertex* cameraEndPrev = t > 1 ? &cameraSubpath[t - 2] : nullptr;
	
	ScopedAssignment<PathVertex> sampledAssign;
	if(s == 1)
		sampledAssign = {lightEnd, sampled};

	ScopedAssignment<bool> deltaAssign1;
	ScopedAssignment<bool> deltaAssign2;
	if(lightEnd)
		deltaAssign1 = {&lightEnd->delta, false};
	if(cameraEnd)
		deltaAssign1 = {&cameraEnd->delta, false};

	ScopedAssignment<float> cameraEndAssign;
	if(cameraEnd)
	{
		if(s > 0)
			cameraEndAssign = {&cameraEnd->pdfRev, lightEnd->pdf(scene, lightEndPrev, *cameraEnd)};
		else
			cameraEndAssign = {&cameraEnd->pdfRev, cameraEnd->pdf_light_origin(scene, *cameraEndPrev)};
	}

	ScopedAssignment<float> cameraEndPrevAssign;
	if(cameraEndPrev)
	{
		if(s > 0)
			cameraEndPrevAssign = {&cameraEndPrev->pdfRev, cameraEnd->pdf(scene, lightEnd, *cameraEndPrev)};
		else
			cameraEndPrevAssign = {&cameraEndPrev->pdfRev, cameraEnd->pdf_light(scene, *cameraEndPrev)};
	}

	ScopedAssignment<float> lightEndAssign;
	if(lightEnd)
		lightEndAssign = {&lightEnd->pdfRev, cameraEnd->pdf(scene, cameraEndPrev, *lightEnd)};

	ScopedAssignment<float> lightEndPrevAssign;
	if(lightEndPrev)
		lightEndPrevAssign = {&lightEndPrev->pdfRev, lightEnd->pdf(scene, cameraEnd, *lightEndPrev)};

	//compute mis weight:
	//---------------
	float sum = 0.0f;

	float pdfCam = 1.0f;
	for(uint32_t i = t - 1; i > 1; i--) //TODO: allow i == 1
	{
		pdfCam *= (cameraSubpath[i].pdfRev == 0.0f ? 1.0f : cameraSubpath[i].pdfRev) / 
		          (cameraSubpath[i].pdfFwd == 0.0f ? 1.0f : cameraSubpath[i].pdfFwd);

		if(!cameraSubpath[i].delta && !cameraSubpath[i - 1].delta)
			sum += pdfCam;
	}

	float pdfLight = 1.0f;
	for(int32_t i = (int32_t)s - 1; i >= 0; i--)
	{
		pdfLight *= (lightSubpath[i].pdfRev == 0.0f ? 1.0f : lightSubpath[i].pdfRev) /
		            (lightSubpath[i].pdfFwd == 0.0f ? 1.0f : lightSubpath[i].pdfFwd);

		bool deltaLight = i > 0 ? lightSubpath[i - 1].delta : lightSubpath[0].intersection.light->is_delta();
		if(!lightSubpath[i].delta && !deltaLight)
			sum += pdfLight;
	}

	return 1.0f / (1.0f + sum);
}

float RendererBidirectional::uniform_weight(std::vector<PathVertex>& cameraSubpath, std::vector<PathVertex>& lightSubpath, uint32_t s, uint32_t t) const
{
	//early return for paths of depth 2:
	//---------------
	if(s + t == 2)
		return 1.0f;

	//remove delta for connected vertices:
	//---------------
	PathVertex* lightEnd      = s > 0 ? &lightSubpath [s - 1] : nullptr;
	PathVertex* cameraEnd     = t > 0 ? &cameraSubpath[t - 1] : nullptr;
	PathVertex* lightEndPrev  = s > 1 ? &lightSubpath [s - 2] : nullptr;
	PathVertex* cameraEndPrev = t > 1 ? &cameraSubpath[t - 2] : nullptr;
	
	ScopedAssignment<bool> deltaAssign1;
	ScopedAssignment<bool> deltaAssign2;
	if(lightEnd)
		deltaAssign1 = {&lightEnd->delta, false};
	if(cameraEnd)
		deltaAssign1 = {&cameraEnd->delta, false};

	//compute mis weight:
	//---------------
	float sum = 0.0f;

	for(uint32_t i = t - 1; i > 1; i--) //TODO: allow i == 1
		if(!cameraSubpath[i].delta && !cameraSubpath[i - 1].delta)
			sum += 1.0f;

	for(int32_t i = (int32_t)s - 1; i >= 0; i--)
	{
		bool deltaLight = i > 0 ? lightSubpath[i - 1].delta : lightSubpath[0].intersection.light->is_delta();
		if(!lightSubpath[i].delta && !deltaLight)
			sum += 1.0f;
	}

	return 1.0f / (1.0f + sum);
}

//-------------------------------------------//

float RendererBidirectional::PathVertex::convert_density(float pdf, const PathVertex& next) const
{
	if(next.type == Type::LIGHT && next.intersection.light->is_infinite())
		return pdf;
	
	vec3 to = next.intersection.pos - intersection.pos;
	float invDist2 = 1.0f / dot(to, to);

	return pdf * std::abs(dot(next.intersection.shadingNormal, to * std::sqrtf(invDist2))) * invDist2;
}

vec3 RendererBidirectional::PathVertex::f(const PathVertex& next) const
{
	vec3 wi = normalize(next.intersection.pos - intersection.pos);
	return intersection.bsdf->f(wi, intersection.wo, BXDFflags::ALL);
}

float RendererBidirectional::PathVertex::pdf(const std::shared_ptr<const Scene>& scene, PathVertex* prev, const PathVertex& next) const
{
	if(type == Type::LIGHT)
		return pdf_light(scene, next);
	
	vec3 n = normalize(next.intersection.pos - intersection.pos);
	vec3 p;
	if(prev)
		p = normalize(prev->intersection.pos - intersection.pos);

	float pdf;
	if(type == Type::CAMERA)
		pdf = 1.0f; //TODO: implement (we shouldnt ever get here currently)
	else if(type == Type::SURFACE)
		pdf = intersection.bsdf->pdf(n, p, BXDFflags::ALL); 

	return convert_density(pdf, next);
}

float RendererBidirectional::PathVertex::pdf_light(const std::shared_ptr<const Scene>& scene, const PathVertex& next) const
{
	vec3 to = next.intersection.pos - intersection.pos;
	float invDist2 = 1.0f / dot(to, to);
	to = to * invDist2;

	float pdf;
	if(intersection.light->is_infinite())
	{
		float worldRadius = scene->get_world_radius();
		pdf = 1.0f / (FR_PI * worldRadius * worldRadius);
	}
	else
	{
		float pdfPos;
		float pdfDir;
		intersection.light->pdf_le(Ray(intersection.pos, to), intersection.shadingNormal, pdfPos, pdfDir);

		pdf = pdfDir * invDist2;
	}

	pdf *= std::abs(dot(next.intersection.shadingNormal, to));
	return pdf;
}

float RendererBidirectional::PathVertex::pdf_light_origin(const std::shared_ptr<const Scene>& scene, const PathVertex& next) const
{
	vec3 to = normalize(next.intersection.pos - intersection.pos);

	float pdf;
	if(intersection.light->is_infinite())
	{
		pdf = 0.0f;
		const std::vector<std::shared_ptr<const Light>>& infLights = scene->get_infinite_lights();
		for(uint32_t i = 0; i < infLights.size(); i++)
			pdf += infLights[i]->pdf_li({}, -1.0f * to);
		
		pdf *= (float)infLights.size() / (float)scene->get_lights().size();
	}
	else
	{
		float pdfPos;
		float pdfDir;

		intersection.light->pdf_le(Ray(intersection.pos, to), intersection.shadingNormal, pdfPos, pdfDir);
		pdf = pdfPos / (float)scene->get_lights().size();
	}

	return pdf;
}

RendererBidirectional::PathVertex RendererBidirectional::PathVertex::from_surface(const IntersectionInfo& surface, const vec3& mult, float pdf, const PathVertex& prev)
{
	PathVertex vert;
	vert.type = Type::SURFACE;
	vert.intersection = surface;

	vert.mult = mult;
	vert.delta = false;
	vert.pdfRev = 0.0f;
	vert.pdfFwd = prev.convert_density(pdf, vert);

	return vert;
}

RendererBidirectional::PathVertex RendererBidirectional::PathVertex::from_light(std::shared_ptr<const Light> light, const IntersectionInfo& hitInfo, const vec3& mult, float pdf)
{
	PathVertex vert;
	vert.type = Type::LIGHT;
	vert.intersection = hitInfo;
	vert.intersection.light = light;

	vert.mult = mult;
	vert.delta = false;
	vert.pdfRev = 0.0f;
	vert.pdfFwd = pdf;

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
	vert.delta = false;
	vert.pdfRev = 0.0f;
	vert.pdfFwd = 0.0f;

	return vert;
}

}; //namespace fr