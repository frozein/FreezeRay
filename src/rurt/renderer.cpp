#include "rurt/renderer.hpp"

#include "rurt/ray.hpp"
#include "rurt/globals.hpp"

#define RURT_RAY_BOUNCE_LIMIT 50

//-------------------------------------------//

namespace rurt
{

//-------------------------------------------//

Renderer::Renderer(std::shared_ptr<const Scene> scene, std::shared_ptr<const Camera> cam, uint32_t imageW, uint32_t imageH, uint32_t spp) : 
	m_scene(scene),
	m_cam(cam),
	m_camInvView(inverse(m_cam->view())),
	m_camInvProj(inverse(m_cam->proj())),
	m_imageW(imageW), 
	m_imageH(imageH),
	m_spp(spp)
{

}

Renderer::~Renderer()
{

}

void Renderer::draw_scanline(uint32_t y, uint32_t* buf)
{
	//loop over every pixel in scanline:
	//---------------
	for(uint32_t x = 0; x < m_imageW; x++)
	{
		//generate ray for current pixel:
		//---------------
		Ray cameraRay = get_camera_ray(x, y);

		//bounce ray until hit sky or bounce limit reached:
		//---------------
		vec3 color = vec3(0.0f);
		for(uint32_t i = 0; i < m_spp; i++)
		{
			vec3 pathColor = trace_path(cameraRay);
			color = color + pathColor / (float)m_spp;
		}

		//write color to given buffer:
		//---------------
		color.r = std::max(std::min(color.r, 1.0f), 0.0f);
		color.g = std::max(std::min(color.g, 1.0f), 0.0f);
		color.b = std::max(std::min(color.b, 1.0f), 0.0f);
		color = linear_to_srgb(color);

		uint8_t r = (uint8_t)(color.r * 255.0f);
		uint8_t g = (uint8_t)(color.g * 255.0f);
		uint8_t b = (uint8_t)(color.b * 255.0f);
		uint8_t a = UINT8_MAX; //each pixel fully opaque

		uint32_t writeColor = ((uint32_t)r << 24) | ((uint32_t)g << 16) | ((uint32_t)b << 8) | (uint32_t)a;
		buf[x] = writeColor;
	}
}

//-------------------------------------------//

vec3 Renderer::trace_path(const Ray& cameraRay) const
{
	vec3 light = vec3(0.0f);
	vec3 mult = vec3(1.0f);

	static int count = 0;
	count++;

	Ray curRay = cameraRay;
	for(uint32_t i = 0; i < RURT_RAY_BOUNCE_LIMIT; i++)
	{
		IntersectionInfo hitInfo;
		bool hit = m_scene->intersect(curRay, hitInfo);

		if(!hit)
		{
			//TODO: add contribution from infinite area lights (environment maps)
			//only do this for rays spawned from delta distribs
			break;
		}

		//negate ray direction to get wo:
		vec3 wo = -1.0f * curRay.direction();

		//add contribution from light sources
		if(!hitInfo.material->bsdf_is_delta())
			light = light + mult * uniform_sample_one_light(hitInfo, wo);

		//evaluate bsdf:
		vec3 f;
		float pdf;
		vec3 wi;

		if(hitInfo.material->bsdf_is_delta())
		{
			vec2 u = vec2((float)rand() / RAND_MAX, (float)rand() / RAND_MAX);
			f = hitInfo.material->bsdf_sample_f(hitInfo, wi, wo, u, pdf);
		}
		else
		{
			switch(hitInfo.material->bsdf_type())
			{
			case BXDFType::REFLECTION:
				wi = random_dir_hemisphere(hitInfo.worldNormal);
				pdf = RURT_INV_2_PI;
				break;
			case BXDFType::TRANSMISSION:
				wi = random_dir_hemisphere(-1.0f * hitInfo.worldNormal);
				pdf = RURT_INV_2_PI;
				break;
			case BXDFType::BOTH:
			default:
				wi = random_dir_sphere();
				pdf = 2.0f * RURT_INV_2_PI;
				break;
			}

			f = hitInfo.material->bsdf_f(hitInfo, wi, wo);
		}

		//apply brdf to current color
		float cosTheta = std::abs(dot(wi, hitInfo.worldNormal));
		mult = mult * (f * cosTheta / pdf);

		//set new ray
		vec3 bounceDir = wi;
		vec3 bouncePos = hitInfo.worldPos;
		
		if(dot(bounceDir, hitInfo.worldNormal) > 0.0f) //reflection
			bouncePos = bouncePos + RURT_EPSILON * hitInfo.worldNormal;
		else //transmission
		{
			bool entering = dot(wo, hitInfo.worldNormal) > 0.0f;
			bouncePos = bouncePos + (entering ? -RURT_EPSILON : RURT_EPSILON) * hitInfo.worldNormal;
		}

		curRay = Ray(bouncePos, bounceDir);

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

vec3 Renderer::uniform_sample_one_light(const IntersectionInfo& hitInfo, const vec3& wo) const
{
	//choose random light index:
	//---------------
	uint32_t numLights = (uint32_t)m_scene->get_lights().size();
	if(numLights == 0)
		return vec3(0.0f);

	uint32_t lightIdx = rand() % numLights;

	//sample li:
	//---------------
	vec2 u = vec2((float)rand() / RAND_MAX, (float)rand() / RAND_MAX);
	vec3 wi;
	VisibilityTestInfo visInfo;
	float pdf;

	const std::shared_ptr<const Light>& light = m_scene->get_lights()[lightIdx];
	vec3 li = light->sample_li(hitInfo, u, wi, visInfo, pdf);

	pdf /= (float)numLights;

	//compute bsdf f:
	//---------------
	vec3 f = hitInfo.material->bsdf_f(hitInfo, wi, wo) * std::abs(dot(wi, hitInfo.worldNormal));

	//trace visibility ray, return:
	//---------------
	if(trace_visibility_ray(hitInfo, wi, wo, visInfo))
		return f * li / pdf;
	else
		return vec3(0.0f);
}

bool Renderer::trace_visibility_ray(const IntersectionInfo& initialHitInfo, const vec3& wi, const vec3& wo, const VisibilityTestInfo& visInfo) const
{
	vec3 rayPos = initialHitInfo.worldPos;
	if(dot(wo, initialHitInfo.worldNormal) > 0.0f)
		rayPos = rayPos + initialHitInfo.worldNormal * RURT_EPSILON;
	else
		rayPos = rayPos - initialHitInfo.worldNormal * RURT_EPSILON;

	Ray ray(rayPos, wi);

	IntersectionInfo hitInfo;
	bool hit = m_scene->intersect(ray, hitInfo);

	if(visInfo.infinite)
		return !hit;
	else
	{
		if(!hit)
			return true;

		float maxDist = distance(initialHitInfo.worldPos, visInfo.endPos);
		float dist = distance(initialHitInfo.worldPos, hitInfo.worldPos);

		return dist > maxDist;
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