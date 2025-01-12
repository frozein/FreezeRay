/* texture_image_float.hpp
 *
 * contains a definition for a 1-component texture sampling an image
 */

#ifndef RURT_TEXTURE_IMAGE_FLOAT_H
#define RURT_TEXTURE_IMAGE_FLOAT_H

#include "../texture.hpp"

//-------------------------------------------//

namespace rurt
{

class TextureImageFloat : public Texture<float>
{
public:
	TextureImageFloat(uint32_t width, uint32_t height, std::unique_ptr<const uint8_t[]> image, TextureRepeatMode repeatMode);

	float evaluate(const IntersectionInfo& hitInfo) const override;

	static std::shared_ptr<TextureImageFloat> from_file(const std::string& path, TextureRepeatMode repeatMode);

private:
	uint32_t m_width;
	uint32_t m_height;
	std::unique_ptr<const uint8_t[]> m_image; //1-byte per component

	TextureRepeatMode m_repeatMode;
};

}; //namespace rurt

#endif //#ifndef RURT_TEXTURE_IMAGE_FLOAT_H