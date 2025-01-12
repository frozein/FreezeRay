#include "rurt/texture/texture_image_vec3.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

//-------------------------------------------//

namespace rurt
{

TextureImageVec3::TextureImageVec3(uint32_t width, uint32_t height, std::unique_ptr<const uint8_t[]> image, TextureRepeatMode repeatMode) :
	m_width(width), m_height(height), m_image(std::move(image)), m_repeatMode(repeatMode)
{

}

vec3 TextureImageVec3::evaluate(const IntersectionInfo& hitInfo) const
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
	idx *= 3;

	vec3 color = vec3(
		m_image[idx + 0] / 255.0f,
		m_image[idx + 1] / 255.0f,
		m_image[idx + 2] / 255.0f
	);

	return color;
}

std::shared_ptr<TextureImageVec3> TextureImageVec3::from_file(const std::string& path, TextureRepeatMode repeatMode)
{
	int width;
	int height;
	int numChannels;
	uint8_t* imageRaw = stbi_load(path.c_str(), &width, &height, &numChannels, 3);
	if(!imageRaw)
		throw std::runtime_error("failed to load image from \"" + path + "\"");

	std::unique_ptr<uint8_t[]> image = std::unique_ptr<uint8_t[]>(new uint8_t[width * height * 3]);
	memcpy(image.get(), imageRaw, width * height * 3 * sizeof(uint8_t));

	stbi_image_free(imageRaw);

	return std::make_shared<TextureImageVec3>((uint32_t)width, (uint32_t)height, std::move(image), repeatMode);
}

std::pair<std::shared_ptr<TextureImageVec3>, std::shared_ptr<TextureImageFloat>> TextureImageVec3::from_file_with_alpha(const std::string& path, TextureRepeatMode repeatMode)
{
	int width;
	int height;
	int numChannels;
	uint8_t* imageRaw = stbi_load(path.c_str(), &width, &height, &numChannels, 4);
	if(!imageRaw)
		throw std::runtime_error("failed to load image from \"" + path + "\"");

	std::unique_ptr<uint8_t[]> image = std::unique_ptr<uint8_t[]>(new uint8_t[width * height * 3]);
	std::unique_ptr<uint8_t[]> imageAlpha = std::unique_ptr<uint8_t[]>(new uint8_t[width * height]);
	for(uint32_t i = 0; i < (uint32_t)(width * height); i++)
	{
		uint32_t idxRaw = i * 4;
		uint32_t idxImg = i * 3;

		image[idxImg + 0] = imageRaw[idxRaw + 0];
		image[idxImg + 1] = imageRaw[idxRaw + 1];
		image[idxImg + 2] = imageRaw[idxRaw + 2];

		imageAlpha[i] = imageRaw[idxRaw + 3];
	}

	stbi_image_free(imageRaw);

	std::shared_ptr<TextureImageVec3> texture = std::make_shared<TextureImageVec3>(
		(uint32_t)width, (uint32_t)height, std::move(image), repeatMode
	);
	std::shared_ptr<TextureImageFloat> textureAlpha = std::make_shared<TextureImageFloat>(
		(uint32_t)width, (uint32_t)height, std::move(imageAlpha), repeatMode
	);

	return { texture, textureAlpha };
}

}; //namespace rurt