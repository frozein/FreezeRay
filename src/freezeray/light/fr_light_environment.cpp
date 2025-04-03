#include "freezeray/light/fr_light_environment.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "freezeray/fr_globals.hpp"
#include "freezeray/texture/stb_image.h"
#include "freezeray/fr_scene.hpp"

//TODO tweak
#define MAX_TEXEL_DIST_RESOLUTION 128u

//-------------------------------------------//

namespace fr
{

LightEnvironment::LightEnvironment(std::shared_ptr<const vec3[]> image, uint32_t width, uint32_t height) :
	Light(false, true), m_image(image), m_width(width), m_height(height), m_worldRadius(1.0f),
	m_distWidth(std::min(width, MAX_TEXEL_DIST_RESOLUTION)), m_distHeight(std::min(height, MAX_TEXEL_DIST_RESOLUTION))
{
	//validate:
	//---------------
	if(!m_image)
		throw std::invalid_argument("image cannot be NULL");

	if(m_width == 0 || m_height == 0)
		throw std::invalid_argument("image dimensions must be positive");

	//generate texel distribution:
	//---------------
	create_texel_distribution();
}

LightEnvironment::LightEnvironment(const std::string& path) :
	Light(false, true), m_worldRadius(1.0f)
{
	//load image:
	//---------------
	stbi_set_flip_vertically_on_load(false);

	int width;
	int height;
	int numChannels;
	float* imageRaw = stbi_loadf(path.c_str(), &width, &height, &numChannels, 3);
	if(!imageRaw)
		throw std::runtime_error("failed to load image from \"" + path + "\"");

	m_image = std::shared_ptr<vec3[]>((vec3*)imageRaw, [](const vec3* img) { stbi_image_free((void*)img); });
	m_width = (uint32_t)width;
	m_height = (uint32_t)height;
	
	m_distWidth = std::min(m_width, MAX_TEXEL_DIST_RESOLUTION);
	m_distHeight = std::min(m_height, MAX_TEXEL_DIST_RESOLUTION);

	//generate texel distribution:
	//---------------
	create_texel_distribution();
}

vec3 LightEnvironment::sample_li(const IntersectionInfo& hitInfo, const vec3& u, vec3& wiWorld, VisibilityTestInfo& vis, float& pdf) const
{
	//sample texel:
	//---------------
	TexelCoordinate texel;
	vec2 uv = sample_texel_area(u, texel, pdf);
	vec3 li = get_texel(texel.u * m_width / m_distWidth, texel.v * m_height / m_distHeight);

	//uv -> spherical:
	//---------------
	float theta = uv.x * FR_2_PI;
	float phi = uv.y * FR_PI;
	float cosTheta = std::cos(theta);
	float sinTheta = std::sin(theta);
	float sinPhi = std::sin(phi);
	float cosPhi = std::cos(phi);

	//return:
	//---------------
	wiWorld = vec3(sinPhi * cosTheta, cosPhi, sinPhi * sinTheta);
	pdf /= (2.0f * FR_PI * FR_PI * sinPhi);
	vis.infinite = true;
	vis.endPos = vec3(0.0f);

	return li;
}

float LightEnvironment::pdf_li(const IntersectionInfo& hitInfo, const vec3& w) const
{
	float theta = std::atan2(w.z, w.x);
	if(theta < 0.0f)
		theta += FR_2_PI;

	float phi = std::acos(w.y);
	float sinPhi = std::sin(phi);
	
	uint32_t u = (uint32_t)((theta * FR_INV_2_PI) * m_distWidth);
	uint32_t v = (uint32_t)((phi * FR_INV_PI) * m_distHeight);
	u = std::min(u, m_distWidth - 1);
	v = std::min(v, m_distHeight - 1);

	float pdf = m_texelDistribution->pdf(u + m_distWidth * v);
	pdf *= m_distWidth * m_distHeight;
	pdf /= (2.0f * FR_PI * FR_PI * sinPhi);

	return pdf;
}

vec3 LightEnvironment::power() const
{
	return FR_PI * m_power * m_worldRadius * m_worldRadius;
}

vec3 LightEnvironment::le(const IntersectionInfo& hitInfo, const vec3& w) const
{
	vec3 li = -1.0f * w;

	float u = std::atan2(li.z, li.x);
	if(u < 0.0f)
		u += FR_2_PI;

	float v = std::acos(li.y);

	return bilinear(vec2(u * FR_INV_2_PI, v * FR_INV_PI));
}

vec3 LightEnvironment::sample_le(const vec3& u1, const vec3& u2, Ray& ray, vec3& normal, float& pdfPos, float& pdfDir) const
{
	//sample texel:
	//---------------
	TexelCoordinate texel;
	vec2 uv = sample_texel_area(u1, texel, pdfDir);
	vec3 le = get_texel(texel.u * m_width / m_distWidth, texel.v * m_height / m_distHeight);

	//uv -> spherical:
	//---------------
	float theta = uv.x * FR_2_PI;
	float phi = uv.y * FR_PI;
	float cosTheta = std::cos(theta);
	float sinTheta = std::sin(theta);
	float sinPhi = std::sin(phi);
	float cosPhi = std::cos(phi);

	vec3 dir = vec3(sinPhi * cosTheta, cosPhi, sinPhi * sinTheta);
	
	//sample point on disk:
	//---------------
	float diskR = std::sqrt(u2.x);
	float diskTheta = FR_2_PI * u2.y;
	vec2 diskPos = vec2(diskR * std::cos(diskTheta), diskR * std::sin(diskTheta));

	vec3 v1, v2;
	get_orthogonal(dir, v1, v2);

	vec3 pos = m_worldRadius * (diskPos.x * v1 + diskPos.y * v2);

	//return:
	//---------------
	ray = Ray(pos + m_worldRadius * dir, -1.0f * dir);
	normal = -1.0f * dir;
	pdfPos = 1.0f / (FR_PI * m_worldRadius * m_worldRadius);
	pdfDir /= (2.0f * FR_PI * FR_PI * sinPhi);
	
	return le;
}

void LightEnvironment::pdf_le(const Ray& ray, const vec3& normal, float& pdfPos, float& pdfDir) const
{
	//TODO
}

//-------------------------------------------//

vec3 LightEnvironment::get_texel(uint32_t u, uint32_t v) const
{
	u = std::min(u, m_width  - 1);
	v = std::min(v, m_height - 1);

	return m_image[u + m_width * v];
}

vec3 LightEnvironment::bilinear(const vec2& uv) const
{
	float u = uv.x * m_width  - 0.5f;
	float v = uv.y * m_height - 0.5f;

	uint32_t u0 = (uint32_t)u;
	uint32_t v0 = (uint32_t)v;

	float du = u - u0;
	float dv = v - v0;

	return (1 - du) * ((1 - dv) * get_texel(u0    , v0) + dv * get_texel(u0    , v0 + 1)) +
	            du  * ((1 - dv) * get_texel(u0 + 1, v0) + dv * get_texel(u0 + 1, v0 + 1));
}

vec2 LightEnvironment::sample_texel_area(const vec3& u, TexelCoordinate& texel, float& pdf) const
{
	texel = m_texelDistribution->sample(u.x, pdf);

	vec2 uv = vec2(
		(texel.u + u.y) / (float)m_distWidth, 
		(texel.v + u.z) / (float)m_distHeight
	);
	pdf *= m_distWidth * m_distHeight;

	return uv;
}

void LightEnvironment::create_texel_distribution()
{
	std::vector<std::pair<TexelCoordinate, float>> pmf(m_distWidth * m_distHeight);	
	m_power = vec3(0.0f);

	for(uint32_t y = 0; y < m_distHeight; y++)
	{
		for(uint32_t x = 0; x < m_distWidth; x++)
		{
			uint32_t startX = ( x      * m_width ) / m_distWidth;
			uint32_t endX   = ((x + 1) * m_width ) / m_distWidth;
			uint32_t startY = ( y      * m_height) / m_distHeight;
			uint32_t endY   = ((y + 1) * m_height) / m_distHeight;
			
			endX = std::min(endX, m_width);
			endY = std::min(endY, m_height);
			
			float totalLum = 0.0f;
			vec3 totalColor = vec3(0.0f);
			uint32_t count = 0;
			
			for(uint32_t imgY = startY; imgY < endY; imgY++)
			{
				float sinTheta = std::sin(FR_PI * ((float)imgY + 0.5f) / (float)m_height);
				
				for(uint32_t imgX = startX; imgX < endX; imgX++)
				{
					vec3 texelValue = m_image[imgX + m_width * imgY];
					float lum = luminance(texelValue) * sinTheta;
					
					totalLum += lum;
					totalColor = totalColor + texelValue;
					count++;
				}
			}
			
			float avgLum = (count > 0) ? (totalLum / count) : 0.0f;
			vec3 avgColor = (count > 0) ? (totalColor / (float)count) : vec3(0.0f);
			
			pmf[x + m_distWidth * y] = { {x, y}, avgLum };
			m_power = m_power + avgColor;
		}
	}

	m_texelDistribution = std::make_unique<DistributionDiscrete<TexelCoordinate>>(pmf);
	m_power = m_power / (float)(m_distWidth * m_distHeight);
}

void LightEnvironment::preprocess(std::shared_ptr<const Scene> scene)
{
	m_worldRadius = scene->get_world_radius();
}

}; // namespace fr