#include "freezeray/light/fr_light_environment.hpp"
#include "freezeray/fr_globals.hpp"
#include "freezeray/texture/stb_image.h"

//-------------------------------------------//

namespace fr
{

LightEnvironment::LightEnvironment(std::unique_ptr<const vec3[]> image, uint32_t width, uint32_t height, float worldRadius) :
	Light(false, true), m_image(std::move(image)), m_width(width), m_height(height), m_worldRadius(worldRadius)
{
	//validate:
	//---------------
	if(!m_image)
		throw std::invalid_argument("image cannot be NULL");

	if(m_width == 0 || m_height == 0)
		throw std::invalid_argument("image dimensions must be positive");

	//generate texel distribution:
	//---------------
	create_distribution();
}

LightEnvironment::LightEnvironment(const std::string& path, float worldRadius) :
	Light(false, true), m_worldRadius(worldRadius)
{
	//load image:
	//---------------
	stbi_set_flip_vertically_on_load(true);

	int width;
	int height;
	int numChannels;
	float* imageRaw = stbi_loadf(path.c_str(), &width, &height, &numChannels, 3);
	if(!imageRaw)
		throw std::runtime_error("failed to load image from \"" + path + "\"");

	//TODO: don't copy this

	m_image = std::unique_ptr<vec3[]>(new vec3[width * height]);
	m_width = (uint32_t)width;
	m_height = (uint32_t)height;

	memcpy((void*)m_image.get(), (void*)imageRaw, width * height * sizeof(vec3));

	stbi_image_free((void*)imageRaw);

	//generate texel distribution:
	//---------------
	create_distribution();
}

vec3 LightEnvironment::sample_li(const IntersectionInfo& hitInfo, const vec3& u, vec3& wiWorld, VisibilityTestInfo& vis, float& pdf) const
{
	//sample texel:
	//---------------
	TexelCoordinate texel = m_texelDistribution->sample(u.x);
	vec2 uv = vec2(
		(texel.u + u.y) / (float)m_width, 
		(texel.v + u.z) / (float)m_height
	);

	//uv -> spherical:
	//---------------
	float theta = uv.x * FR_2_PI;
	float phi = uv.y * FR_PI;
	float cosTheta = std::cos(theta);
	float sinTheta = std::sin(theta);
	float sinPhi = std::sin(phi);
	float cosPhi = std::cos(phi);

	//get texel value:
	//---------------
	vec3 li = get_texel(texel.u, texel.v) * sinPhi;
	float lum = luminance(li);

	//return:
	//---------------
	wiWorld = vec3(sinPhi * cosTheta, cosPhi, sinPhi * sinTheta);
	pdf = lum / m_luminance;
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
	
	uint32_t u = (uint32_t)((theta * FR_INV_2_PI) * m_width);
	uint32_t v = (uint32_t)((theta * FR_INV_2_PI) * m_height);

	vec3 li = get_texel(u, v) * std::sin(phi);
	float lum = luminance(li);

	return lum / m_luminance;
}

vec3 LightEnvironment::power() const
{
	return FR_PI * m_power * m_worldRadius * m_worldRadius;
}

vec3 LightEnvironment::le(const IntersectionInfo& hitInfo, const vec3& w) const
{
	float u = std::atan2(w.z, w.x);
	if(u < 0.0f)
		u += FR_2_PI;
	
	float v = std::acos(w.y);

	return bilinear(vec2(u * FR_INV_2_PI, v * FR_INV_PI));
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

void LightEnvironment::create_distribution()
{
	std::vector<std::pair<TexelCoordinate, float>> pmf(m_width * m_height);
	
	float area = 0.0f;

	m_luminance = 0.0f;
	m_power = vec3(0.0f);

	for(uint32_t y = 0; y < m_height; y++)
	{
		float sinTheta = std::sin(FR_PI * ((float)y + 0.5f) / (float)m_height);

		for(uint32_t x = 0; x < m_width; x++)
		{
			vec3 texelValue = m_image[x + m_width * y];
			texelValue = texelValue * sinTheta;
			float lum = luminance(texelValue);

			pmf[x + m_width * y] = { {x, y}, lum };

			m_luminance += lum;
			m_power = m_power + texelValue;
		}

		area += m_width * sinTheta;
	}

	m_texelDistribution = std::make_unique<Distribution<TexelCoordinate>>(pmf, m_luminance);

	for(uint32_t i = 0; i < 1000; i++)
	{
		float test = (float)rand() / RAND_MAX;
		
	}

	m_luminance /= area;
	m_power = m_power / area;
}

}; //namespace fr