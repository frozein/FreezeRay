#include "rurt/texture/texture_image_float.hpp"

#include "stb_image.h"

//-------------------------------------------//

namespace rurt
{

TextureImageFloat::TextureImageFloat(uint32_t width, uint32_t height, std::unique_ptr<const uint8_t[]> image, TextureRepeatMode repeatMode) :
	m_width(width), m_height(height), m_image(std::move(image)), m_repeatMode(repeatMode)
{

}

float TextureImageFloat::evaluate(const IntersectionInfo& hitInfo) const
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

	return m_image[idx] / 255.0f;
}

std::shared_ptr<TextureImageFloat> TextureImageFloat::from_file(const std::string& path, TextureRepeatMode repeatMode)
{
	int width;
	int height;
	int numChannels;
	uint8_t* imageRaw = stbi_load(path.c_str(), &width, &height, &numChannels, 1);
	if(!imageRaw)
		throw std::runtime_error("failed to load image from \"" + path + "\"");

	std::unique_ptr<uint8_t[]> image = std::unique_ptr<uint8_t[]>(new uint8_t[width * height]);
	memcpy(image.get(), imageRaw, width * height * sizeof(uint8_t));

	stbi_image_free(imageRaw);

	return std::make_shared<TextureImageFloat>((uint32_t)width, (uint32_t)height, std::move(image), repeatMode);
}

}; //namespace rurt