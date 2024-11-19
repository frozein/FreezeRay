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

		uint8_t r = (uint8_t)(color.r * 255.0f);
		uint8_t g = (uint8_t)(color.g * 255.0f);
		uint8_t b = (uint8_t)(color.b * 255.0f);
		uint8_t a = UINT8_MAX; //each pixel fully opaque

		uint32_t writeColor = ((uint32_t)r << 24) | ((uint32_t)g << 16) | ((uint32_t)b << 8) | (uint32_t)a;
		buf[x] = writeColor;
	}
}

//-------------------------------------------//

vec3 Renderer::trace_path(const Ray& cameraRay)
{
	vec3 color = vec3(1.0f);

	static int count = 0;
	count++;

	Ray curRay = cameraRay;
	for(uint32_t i = 0; i < RURT_RAY_BOUNCE_LIMIT; i++)
	{
		std::shared_ptr<const Material> hitMaterial;
		RaycastInfo info = m_scene->intersect(curRay, hitMaterial);

		if(hitMaterial == nullptr)
		{
			if(i == 0)
				color = info.missInfo.skyColor;
			else
				color = color * info.missInfo.skyEmission;
			
			break;
		}
		else
		{
			//get brdf
			std::shared_ptr<const BXDF> brdf = hitMaterial->get_bxdf();

			//transform current ray direction to local space:
			mat3 toLocal = transform_between(info.hitInfo.worldNormal, RURT_UP_DIR);
			mat3 toWorld = transform_between(RURT_UP_DIR, info.hitInfo.worldNormal);

			//TODO: figure out an actual solution!!!!
			vec3 wo = -1.0f * (toLocal * curRay.direction());
			if(cos_theta(wo) < RURT_EPSILON)
				wo = normalize(vec3(wo.x, RURT_EPSILON, wo.z));

			//evaluate brdf:
			vec3 f;
			float pdf;
			vec3 wi;

			if(brdf->is_delta())
			{
				f = brdf->sample_f(info.hitInfo, wi, wo, pdf);
			}
			else
			{
				wi = random_dir_hemisphere();
				f = brdf->f(info.hitInfo, wi, wo);
				pdf = RURT_INV_2_PI;
			}

			//apply brdf to current color
			float cosTheta = std::max(cos_theta(wi), 0.0f);
			color = color * (f * cosTheta / pdf);

			//set new ray
			vec3 bounceDir = toWorld * wi;
			vec3 bouncePos = info.hitInfo.worldPos + RURT_EPSILON * info.hitInfo.worldNormal;

			curRay = Ray(bouncePos, bounceDir);
		}

		//russian roulette to exit based on color:
		float maxComp = std::max(std::max(color.r, color.g), color.b);
		float roulette = (float)rand() / RAND_MAX;
		if(roulette > maxComp)
			break;
	}

	color.r = std::min(std::max(color.r, 0.0f), 1.0f);
	color.g = std::min(std::max(color.g, 0.0f), 1.0f);
	color.b = std::min(std::max(color.b, 0.0f), 1.0f);

	return linear_to_srgb(color);
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

vec3 Renderer::random_dir_hemisphere()
{
	vec3 randUnitSphere;
	while(true)
	{
		randUnitSphere.x = ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f;
		randUnitSphere.y = ((float)rand() / (float)RAND_MAX);
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

mat3 Renderer::transform_between(const vec3& from, const vec3& to) 
{
	//equal
	if(dot(from, to) >=  1.0f - RURT_EPSILON)
		return mat3_identity();

	//opposite
	if(dot(from, to) <= -1.0f + RURT_EPSILON)
	{
		mat3 mat = mat3();
		mat[0][0] = -1.0f;
		mat[1][1] = -1.0f;
		mat[2][2] = 1.0f;
	}

    vec3 v = cross(from, to);
    float c = dot(from, to);
    float k = 1.0f / (1.0f + c);
    
    mat3 rotation;

    rotation.v[0] = vec3(
        v.x * v.x * k + c,
        v.y * v.x * k + v.z,
        v.z * v.x * k - v.y
    );
    
    rotation.v[1] = vec3(
        v.x * v.y * k - v.z,
        v.y * v.y * k + c,
        v.z * v.y * k + v.x
    );
    
    rotation.v[2] = vec3(
        v.x * v.z * k + v.y,
        v.y * v.z * k - v.x,
        v.z * v.z * k + c
    );
    
    return rotation;
}

}; //namespace rurt