/* texture_image.hpp
 *
 * contains a definition for a texture sampling an image
 */

#ifndef RURT_TEXTURE_IMAGE_H
#define RURT_TEXTURE_IMAGE_H

#include "../texture.hpp"

//-------------------------------------------//

namespace rurt
{

enum class TextureRepeatMode
{
	REPEAT,
	CLAMP_TO_EDGE
};

template<typename T>
class TextureImage : public Texture<T>
{
public:
	TextureImage(uint32_t width, uint32_t height, std::unique_ptr<const T[]> image, TextureRepeatMode repeatMode);

	T evaluate(const IntersectionInfo& hitInfo) const override;

private:
	uint32_t m_width;
	uint32_t m_height;
	std::unique_ptr<const T[]> m_image;

	TextureRepeatMode m_repeatMode;
};

class TextureImageLoader
{
public:
	static std::shared_ptr<TextureImage<float>> float_from_file(const std::string& path, TextureRepeatMode repeatMode);
	static std::shared_ptr<TextureImage<vec3>> vec3_from_file(const std::string& path, TextureRepeatMode repeatMode);
	static std::pair<std::shared_ptr<TextureImage<vec3>>, std::shared_ptr<TextureImage<float>>>
		vec3_from_file_with_alpha(const std::string& path, TextureRepeatMode repeatMode);
};

}; //namespace rurt

#endif //#ifndef RURT_TEXTURE_IMAGE_H