/* texture_image_vec3.hpp
 *
 * contains a definition for a 3-component texture sampling an image
 */

#ifndef RURT_TEXTURE_IMAGE_VEC3_H
#define RURT_TEXTURE_IMAGE_VEC3_H

#include "../texture.hpp"
#include "texture_image_float.hpp"

//-------------------------------------------//

namespace rurt
{

class TextureImageVec3 : public Texture<vec3>
{
public:
	TextureImageVec3(uint32_t width, uint32_t height, std::unique_ptr<const uint8_t[]> image, TextureRepeatMode repeatMode);

	vec3 evaluate(const IntersectionInfo& hitInfo) const override;

	static std::shared_ptr<TextureImageVec3> from_file(const std::string& path, TextureRepeatMode repeatMode);
	static std::pair<std::shared_ptr<TextureImageVec3>, std::shared_ptr<TextureImageFloat>> from_file_with_alpha(const std::string& path, TextureRepeatMode repeatMode);

private:
	uint32_t m_width;
	uint32_t m_height;
	std::unique_ptr<const uint8_t[]> m_image; //1-byte per component

	TextureRepeatMode m_repeatMode;
};

}; //namespace rurt

#endif //#ifndef RURT_TEXTURE_IMAGE_VEC3_H