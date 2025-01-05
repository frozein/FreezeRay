#include "rurt/texture/texture_image.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

//-------------------------------------------//

namespace rurt
{

template<typename T>
TextureImage<T>::TextureImage(uint32_t width, uint32_t height, std::unique_ptr<const T[]> image, TextureRepeatMode repeatMode) :
	m_width(width), m_height(height), m_image(std::move(image)), m_repeatMode(repeatMode)
{

}

template<typename T>
T TextureImage<T>::evaluate(const IntersectionInfo& hitInfo) const
{
	vec2 uv = hitInfo.uv;
	switch(m_repeatMode)
	{
	case TextureRepeatMode::REPEAT:
		uv.x = fmodf(uv.x, 1.0f);
		uv.y = fmodf(uv.y, 1.0f);
		break;
	case TextureRepeatMode::CLAMP_TO_EDGE:
		uv.x = std::min(std::max(uv.x, 0.0f), 1.0f);
		uv.y = std::min(std::max(uv.y, 0.0f), 1.0f);
		break;
	default:
		throw std::invalid_argument("invalid repeat mode");
	}

	//TODO: add trilinear interpolation

	uint32_t x = (uint32_t)(uv.x * m_width);
	uint32_t y = (uint32_t)(uv.y * m_height);
	x = x >= m_width  ? (m_width  - 1) : x;
	y = y >= m_height ? (m_height - 1) : y;

	uint32_t idx = x + m_width * y;
	return m_image[idx];
}

//-------------------------------------------//

std::shared_ptr<TextureImage<float>> TextureImageLoader::float_from_file(const std::string& path, TextureRepeatMode repeatMode)
{
	int width;
	int height;
	int numChannels;
	uint8_t* imageRaw = stbi_load(path.c_str(), &width, &height, &numChannels, 1);
	if(!imageRaw)
		throw std::runtime_error("failed to load image from \"" + path + "\"");

	std::unique_ptr<float[]> image = std::unique_ptr<float[]>(new float[width * height]);
	for(uint32_t i = 0; i < (uint32_t)(width * height); i++)
		image[i] = (float)imageRaw[i] / 255.0f;

	stbi_image_free(imageRaw);

	return std::make_shared<TextureImage<float>>((uint32_t)width, (uint32_t)height, std::move(image), repeatMode);
}

std::shared_ptr<TextureImage<vec3>> TextureImageLoader::vec3_from_file(const std::string& path, TextureRepeatMode repeatMode)
{
	int width;
	int height;
	int numChannels;
	uint8_t* imageRaw = stbi_load(path.c_str(), &width, &height, &numChannels, 3);
	if(!imageRaw)
		throw std::runtime_error("failed to load image from \"" + path + "\"");

	std::unique_ptr<vec3[]> image = std::unique_ptr<vec3[]>(new vec3[width * height]);
	for(uint32_t i = 0; i < (uint32_t)(width * height); i++)
	{
		uint32_t idx = i * 3;
		image[i].x = (float)imageRaw[idx + 0] / 255.0f;
		image[i].y = (float)imageRaw[idx + 1] / 255.0f;
		image[i].z = (float)imageRaw[idx + 2] / 255.0f;
	}

	stbi_image_free(imageRaw);

	return std::make_shared<TextureImage<vec3>>((uint32_t)width, (uint32_t)height, std::move(image), repeatMode);
}

std::pair<std::shared_ptr<TextureImage<vec3>>, std::shared_ptr<TextureImage<float>>> TextureImageLoader::vec3_from_file_with_alpha(const std::string& path, TextureRepeatMode repeatMode)
{
	int width;
	int height;
	int numChannels;
	uint8_t* imageRaw = stbi_load(path.c_str(), &width, &height, &numChannels, 4);
	if(!imageRaw)
		throw std::runtime_error("failed to load image from \"" + path + "\"");

	std::unique_ptr<vec3[]> image = std::unique_ptr<vec3[]>(new vec3[width * height]);
	std::unique_ptr<float[]> imageAlpha = std::unique_ptr<float[]>(new float[width * height]);
	for(uint32_t i = 0; i < (uint32_t)(width * height); i++)
	{
		uint32_t idx = i * 4;
		image[i].x = (float)imageRaw[idx + 0] / 255.0f;
		image[i].y = (float)imageRaw[idx + 1] / 255.0f;
		image[i].z = (float)imageRaw[idx + 2] / 255.0f;

		imageAlpha[i] = (float)imageRaw[idx + 3] / 255.0f;
	}

	stbi_image_free(imageRaw);

	std::shared_ptr<TextureImage<vec3>> texture = std::make_shared<TextureImage<vec3>>(
		(uint32_t)width, (uint32_t)height, std::move(image), repeatMode
	);
	std::shared_ptr<TextureImage<float>> textureAlpha = std::make_shared<TextureImage<float>>(
		(uint32_t)width, (uint32_t)height, std::move(imageAlpha), repeatMode
	);

	return { texture, textureAlpha };
}

}; //namespace rurt