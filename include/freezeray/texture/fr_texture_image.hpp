/* fr_texture_image.hpp
 *
 * contains a definition for a texture sampling an image
 */

#ifndef FR_TEXTURE_IMAGE_H
#define FR_TEXTURE_IMAGE_H

#include "../fr_texture.hpp"

//-------------------------------------------//

namespace fr
{

template<typename T>
struct Image
{
	uint32_t width;
	uint32_t height;
	std::shared_ptr<const T[]> buf;	
};

template<typename T, typename Tmemory>
class TextureImage : public Texture<T>
{
public:
	TextureImage(Image<Tmemory> image, TextureRepeatMode repeatMode, T multiplier = 1.0);
	TextureImage(const std::vector<Image<Tmemory>>& mipPyramid, TextureRepeatMode repeatMode, T multiplier = 1.0);

	T evaluate(const IntersectionInfo& hitInfo) const override;

	static std::shared_ptr<TextureImage<T, Tmemory>> from_file(const std::string& path, bool hdr, TextureRepeatMode repeatMode, T multiplier = 1.0);

	const std::vector<Image<Tmemory>>& get_mip_pyramid();
	TextureRepeatMode get_repeat_mode();

private:
	std::vector<Image<Tmemory>> m_mipPyramid;
	TextureRepeatMode m_repeatMode;
	T m_multiplier;

	//-------------------------------------------//

	inline T get_texel(uint32_t level, int32_t u, int32_t v) const;
	inline T bilinear(uint32_t level, const vec2& uv) const;

	static Image<Tmemory> resize_to_power_of_2(Image<Tmemory> image, TextureRepeatMode repeatMode);
	static std::pair<std::unique_ptr<int32_t[]>, std::unique_ptr<float[]>> compute_resampling_weights(uint32_t oldSize, uint32_t newSize);
	static float lanczos_filter(float x);
};

//-------------------------------------------//

inline void convert_from_texture_memory(uint32_t mem, vec3& val);
inline void convert_to_texture_memory(vec3 val, uint32_t& mem);

inline void convert_from_texture_memory(uint8_t mem, float& val);
inline void convert_to_texture_memory(float val, uint8_t& mem);

inline void convert_from_texture_memory(uint32_t mem, float& val);
inline void convert_to_texture_memory(float val, uint32_t& mem);

}; //namespace fr

//include templated definitions
#include "fr_texture_image.tpp"

#endif //#ifndef FR_TEXTURE_IMAGE_H