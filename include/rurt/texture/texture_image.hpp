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

template<typename T, typename Tmemory>
class TextureImage : public Texture<T>
{
public:
	TextureImage(uint32_t width, uint32_t height, std::unique_ptr<const Tmemory[]> image, TextureRepeatMode repeatMode);

	T evaluate(const IntersectionInfo& hitInfo) const override;

	//loads sizeof(Tmemory) components from an image using stb_image, 1 byte-per-component
	static std::shared_ptr<TextureImage<T, Tmemory>> from_file(const std::string& path, TextureRepeatMode repeatMode);

private:
	struct Image
	{
		uint32_t width;
		uint32_t height;
		std::unique_ptr<const Tmemory[]> image;
	};

	std::vector<Image> m_mipPyramid;
	TextureRepeatMode m_repeatMode;

	//-------------------------------------------//

	inline T get_texel(uint32_t level, uint32_t u, uint32_t v) const;
	inline T bilinear(uint32_t level, const vec2& uv) const;

	static Image resize_to_power_of_2(uint32_t width, uint32_t height, const Tmemory* image, TextureRepeatMode repeatMode);
	static std::pair<std::unique_ptr<int32_t[]>, std::unique_ptr<float[]>> compute_resampling_weights(uint32_t oldSize, uint32_t newSize);
	static float lanczos_filter(float x);
};

inline void convert_from_texture_memory(uint32_t mem, vec3& val);
inline void convert_to_texture_memory(vec3 val, uint32_t& mem);

inline void convert_from_texture_memory(uint8_t mem, float& val);
inline void convert_to_texture_memory(float val, uint8_t& mem);

}; //namespace rurt

//include templated definitions
#include "texture_image.tpp"

#endif //#ifndef RURT_TEXTURE_IMAGE_H