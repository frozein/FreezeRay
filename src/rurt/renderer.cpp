#include "rurt/renderer.hpp"

#include "rurt/ray.hpp"
#include "rurt/globals.hpp"
#include <math.h>

#define RURT_RAY_BOUNCE_LIMIT 50

//-------------------------------------------//

namespace rurt
{

//-------------------------------------------//

Renderer::Renderer(const std::shared_ptr<const Camera>& cam, uint32_t imageW, uint32_t imageH) : 
	m_cam(cam),
	m_camInvView(inverse(m_cam->view())),
	m_camInvProj(inverse(m_cam->proj())),
	m_imageW(imageW), 
	m_imageH(imageH)
{

}

Renderer::~Renderer()
{

}

void Renderer::render(const std::shared_ptr<const Scene>& scene, std::function<void(uint32_t, uint32_t, vec3)> writePixel, std::function<void()> display)
{
	//render from top -> bottom (looks more natural)
	for(uint32_t y = m_imageH - 1; y >= 0; y--)
	{
		for(uint32_t x = 0; x < m_imageW; x++)
		{
			//generate ray for current pixel:
			//---------------
			Ray cameraRay = get_camera_ray(x, y);
			Ray cameraRayDifferentialX = get_camera_ray(x + 1, y);
			Ray cameraRayDifferentialY = get_camera_ray(x, y + 1);

			cameraRay = Ray(cameraRay, cameraRayDifferentialX, cameraRayDifferentialY);

			//get color of pixel:
			//---------------
			vec3 color = li(scene, cameraRay);

			//write color to given buffer:
			//---------------
			color.r = std::max(std::min(color.r, 1.0f), 0.0f);
			color.g = std::max(std::min(color.g, 1.0f), 0.0f);
			color.b = std::max(std::min(color.b, 1.0f), 0.0f);
			color = linear_to_srgb(color);

			writePixel(x, y, color);
		}

		//display after each scanline
		//---------------
		display();
	}
}

//-------------------------------------------//

vec3 Renderer::uniform_sample_one_light(const std::shared_ptr<const Scene>& scene, const IntersectionInfo& hitInfo, const vec3& wo) const
{
	//choose random light index:
	//---------------
	uint32_t numLights = (uint32_t)scene->get_lights().size();
	if(numLights == 0)
		return vec3(0.0f);

	uint32_t lightIdx = rand() % numLights;

	//sample li:
	//---------------
	vec3 u = vec3((float)rand() / RAND_MAX, (float)rand() / RAND_MAX, (float)rand() / RAND_MAX);
	vec3 wi;
	VisibilityTestInfo visInfo;
	float pdf;

	const std::shared_ptr<const Light>& light = scene->get_lights()[lightIdx];
	vec3 li = light->sample_li(hitInfo, u, wi, visInfo, pdf);

	pdf /= (float)numLights;

	//compute bsdf f:
	//---------------
	vec3 f = hitInfo.material->bsdf_f(hitInfo, wi, wo) * std::abs(dot(wi, hitInfo.worldNormal));

	//trace visibility ray, return:
	//---------------
	if(trace_visibility_ray(scene, hitInfo, wi, wo, visInfo))
		return f * li / pdf;
	else
		return vec3(0.0f);
}

bool Renderer::trace_visibility_ray(const std::shared_ptr<const Scene>& scene, const IntersectionInfo& initialHitInfo, const vec3& wi, const vec3& wo, const VisibilityTestInfo& visInfo) const
{
	vec3 rayPos = initialHitInfo.worldPos;
	if(dot(wo, initialHitInfo.worldNormal) > 0.0f)
		rayPos = rayPos + initialHitInfo.worldNormal * RURT_EPSILON;
	else
		rayPos = rayPos - initialHitInfo.worldNormal * RURT_EPSILON;

	Ray ray;
	if(visInfo.infinite)
		ray = Ray(rayPos, wi);
	else
	{
		vec3 toEnd = visInfo.endPos - rayPos;
		ray = Ray(rayPos, toEnd);
	}

	IntersectionInfo hitInfo;
	bool hit = scene->intersect(ray, hitInfo);

	if(visInfo.infinite)
		return !hit;
	else
	{
		if(!hit)
			return true;

		float minDist = distance(initialHitInfo.worldPos, visInfo.endPos);
		float dist = distance(initialHitInfo.worldPos, hitInfo.worldPos);

		return (dist + RURT_EPSILON) > minDist;
	}
}

Ray Renderer::get_camera_ray(uint32_t x, uint32_t y) const
{
	vec2 pixelCenter = vec2((float)x, (float)y) + vec2(0.5f);
	vec2 pixelUV = pixelCenter / vec2((float)m_imageW, (float)m_imageH);
	vec2 pixelD = pixelUV * 2.0f - vec2(1.0f);

	vec4 rayOrig = m_camInvView * vec4(0.0f, 0.0f, 0.0f, 1.0f);
	vec4 rayTarget = m_camInvProj * vec4(pixelD.x, pixelD.y, 1.0f, 1.0f);
	vec4 rayDir = m_camInvView * vec4(normalize(rayTarget.xyz()), 0.0f);

	return Ray(rayOrig.xyz(), normalize(rayDir.xyz()));	
}

vec3 Renderer::random_dir_sphere()
{
	vec3 randUnitSphere;
	while(true)
	{
		randUnitSphere.x = ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f;
		randUnitSphere.y = ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f;
		randUnitSphere.z = ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f;

		float lenSqr = dot(randUnitSphere, randUnitSphere);
		if(lenSqr > RURT_EPSILON && lenSqr <= 1.0f)
		{
			randUnitSphere = randUnitSphere / std::sqrtf(lenSqr);
			break;
		}
	}

	return randUnitSphere;
}

vec3 Renderer::random_dir_hemisphere(const vec3& normal)
{
	vec3 randUnitSphere;
	while(true)
	{
		randUnitSphere.x = ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f;
		randUnitSphere.y = ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f;
		randUnitSphere.z = ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f;

		float lenSqr = dot(randUnitSphere, randUnitSphere);
		if(lenSqr > RURT_EPSILON && lenSqr <= 1.0f)
		{
			randUnitSphere = randUnitSphere / std::sqrtf(lenSqr);
			break;
		}
	}

	if(dot(randUnitSphere, normal) < 0.0f)
		randUnitSphere = randUnitSphere * -1.0f;

	return randUnitSphere;
}

}; //namespace rurt