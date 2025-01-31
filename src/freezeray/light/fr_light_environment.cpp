#include "freezeray/light/fr_light_environment.hpp"
#include "freezeray/fr_globals.hpp"
#include "freezeray/texture/stb_image.h"

//-------------------------------------------//

namespace fr
{

LightEnvironment::LightEnvironment(std::unique_ptr<const vec3[]> image, uint32_t width, uint32_t height, float worldRadius) :
	Light(false, true), m_image(std::move(image)), m_width(width), m_height(height), m_power(1.0f), m_worldRadius(worldRadius)
{
	//TODO: initialize structure for sampling
	//TODO: average luminance to get total intensity
}

LightEnvironment::LightEnvironment(const std::string& path, float worldRadius) :
	Light(false, true), m_power(1.0f), m_worldRadius(worldRadius)
{
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
}

vec3 LightEnvironment::sample_li(const IntersectionInfo& hitInfo, const vec3& u, vec3& wiWorld, VisibilityTestInfo& vis, float& pdf) const
{
	//TODO

	wiWorld = vec3(1.0f);
	pdf = 1.0f;
	vis.infinite = true;
	vis.endPos = vec3(0.0f);

	return vec3(0.0f);
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

}; //namespace fr