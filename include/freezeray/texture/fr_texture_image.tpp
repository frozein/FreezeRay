#include "../texture/fr_texture_image.hpp"

#include "stb_image.h"
#include "../fr_globals.hpp"
#include <math.h>

//-------------------------------------------//

namespace fr
{

template<typename T, typename Tmemory, typename Tprocessing>
TextureImage<T, Tmemory, Tprocessing>::TextureImage(Image<Tmemory> image, TextureRepeatMode repeatMode, T multiplier) : 
	m_repeatMode(repeatMode), m_multiplier(multiplier)
{
	//determine if sizes are power of 2, resize if not:
	//---------------
	bool isPowerOf2 = true;

	uint32_t widthTemp = image.get_width();
	while(widthTemp != 1)
	{
		isPowerOf2 = isPowerOf2 && !(widthTemp & 1);
		widthTemp >>= 1;
	}

	uint32_t heightTemp = image.get_height();
	while(heightTemp != 1)
	{
		isPowerOf2 = isPowerOf2 && !(heightTemp & 1);
		heightTemp >>= 1;
	}

	if(!isPowerOf2)
		image = resize_to_power_of_2(image, repeatMode);

	m_mipPyramid.push_back(image);

	//construct mip pyramid:
	//---------------
	uint32_t maxRes = std::max(image.get_width(), image.get_width());
	uint32_t numLevels = 1;
	while(maxRes != 1)
	{
		maxRes >>= 1;
		numLevels++;
	}

	for(uint32_t i = 1; i < numLevels; i++)
	{
		uint32_t levelWidth  = std::max(1u, m_mipPyramid[i - 1].get_width()  / 2);
		uint32_t levelHeight = std::max(1u, m_mipPyramid[i - 1].get_height() / 2);

		Image<Tmemory> levelImage(levelWidth, levelHeight, nullptr);
		
		for(int32_t y = 0; y < (int32_t)levelHeight; y++)
		for(int32_t x = 0; x < (int32_t)levelWidth; x++)
		{
			Tprocessing sample = (
				get_texel_processing(i - 1, 2 * x, 2 * y    ) + get_texel_processing(i - 1, 2 * x + 1, 2 * y    ) +
				get_texel_processing(i - 1, 2 * x, 2 * y + 1) + get_texel_processing(i - 1, 2 * x + 1, 2 * y + 1)
			) / 4.0f;

			Tmemory sampleMem;
			convert_to_texture_memory(sample, sampleMem);

			levelImage.put(x, y, sampleMem);
		}
		
		m_mipPyramid.push_back(levelImage);
	}
}

template<typename T, typename Tmemory, typename Tprocessing>
TextureImage<T, Tmemory, Tprocessing>::TextureImage(const std::vector<Image<Tmemory>>& mipPyramid, TextureRepeatMode repeatMode, T multiplier) :
	m_mipPyramid(mipPyramid), m_repeatMode(repeatMode), m_multiplier(multiplier)
{
	
}

template<typename T, typename Tmemory, typename Tprocessing>
T TextureImage<T, Tmemory, Tprocessing>::evaluate(const IntersectionInfo& hitInfo) const
{
	//TODO: anisotropic sampling

	float width = std::max(
		std::max(std::abs(hitInfo.derivatives.duvdx.x), std::abs(hitInfo.derivatives.duvdx.y)),
		std::max(std::abs(hitInfo.derivatives.duvdy.x), std::abs(hitInfo.derivatives.duvdy.y))
	);

	float level = m_mipPyramid.size() - 1 + std::log2f(std::max(width, 1e-10f));

	T sampled;
	if(level < 0.0f)
		sampled = bilinear(0, hitInfo.uv);
	else if(level >= m_mipPyramid.size() - 1)
		sampled = get_texel((uint32_t)m_mipPyramid.size() - 1, 0, 0);
	else
	{
		uint32_t level0 = (uint32_t)level;
		float dl = level - level0;

		sampled = (1.0f - dl) * bilinear(level0, hitInfo.uv) + dl * bilinear(level0 + 1, hitInfo.uv);
	}

	return sampled * m_multiplier;
}

template<typename T, typename Tmemory, typename Tprocessing>
std::shared_ptr<TextureImage<T, Tmemory, Tprocessing>> TextureImage<T, Tmemory, Tprocessing>::from_file(const std::string& path, bool hdr, TextureRepeatMode repeatMode, T multiplier)
{
	static_assert(sizeof(Tmemory) <= 4, "cannot load an image with >4 components");

	//load image from stbi:
	//---------------
	stbi_set_flip_vertically_on_load(true);

	int width;
	int height;
	int numChannels;
	uint8_t* rawBuf;
	if(hdr)
	{
		rawBuf = (uint8_t*)stbi_loadf(path.c_str(), &width, &height, &numChannels, sizeof(Tmemory) / sizeof(float));
		if(!rawBuf)
			throw std::runtime_error("failed to load image from \"" + path + "\"");
	}
	else
	{
		rawBuf = stbi_load(path.c_str(), &width, &height, &numChannels, sizeof(Tmemory));
		if(!rawBuf)
			throw std::runtime_error("failed to load image from \"" + path + "\"");
	}

	std::shared_ptr<Tmemory[]> buf = std::shared_ptr<Tmemory[]>((Tmemory*)rawBuf, [](const Tmemory* img){ stbi_image_free((void*)img); });

	//resize to power of 2:
	//---------------
	bool isPowerOf2 = true;

	uint32_t widthTemp = (uint32_t)width;
	while(widthTemp != 1)
	{
		isPowerOf2 = isPowerOf2 && !(widthTemp & 1);
		widthTemp >>= 1;
	}

	uint32_t heightTemp = (uint32_t)height;
	while(heightTemp != 1)
	{
		isPowerOf2 = isPowerOf2 && !(heightTemp & 1);
		heightTemp >>= 1;
	}

	Image<Tmemory> image(width, height, buf);

	if(!isPowerOf2)
		image = resize_to_power_of_2(image, repeatMode);

	//return:
	//---------------
	return std::make_shared<TextureImage<T, Tmemory, Tprocessing>>(image, repeatMode, multiplier);
}

template<typename T, typename Tmemory, typename Tprocessing>
const std::vector<Image<Tmemory>>& TextureImage<T, Tmemory, Tprocessing>::get_mip_pyramid()
{
	return m_mipPyramid;
}

template<typename T, typename Tmemory, typename Tprocessing>
TextureRepeatMode TextureImage<T, Tmemory, Tprocessing>::get_repeat_mode()
{
	return m_repeatMode;
}

//-------------------------------------------//

template<typename T, typename Tmemory, typename Tprocessing>
inline T TextureImage<T, Tmemory, Tprocessing>::get_texel(uint32_t level, int32_t u, int32_t v) const
{
	const Image<Tmemory>& image = m_mipPyramid[level];

	switch(m_repeatMode)
	{
	case TextureRepeatMode::REPEAT:
		u = (u % image.get_width()  + image.get_width() ) % image.get_width();
		v = (v % image.get_height() + image.get_height()) % image.get_height();
		break;
	case TextureRepeatMode::CLAMP_TO_EDGE:
		u = std::min(std::max(u, 0), (int32_t)image.get_width()  - 1);
		v = std::min(std::max(v, 0), (int32_t)image.get_height() - 1);
		break;
	default:
		throw std::invalid_argument("invalid repeat mode");
	}

	Tmemory sampleMem = image.get(u, v);

	T sample;
	convert_from_texture_memory(sampleMem, sample);

	return sample;
}

template<typename T, typename Tmemory, typename Tprocessing>
inline Tprocessing TextureImage<T, Tmemory, Tprocessing>::get_texel_processing(uint32_t level, int32_t u, int32_t v) const
{
	const Image<Tmemory>& image = m_mipPyramid[level];
	
	Tmemory sampleMem = image.get(u, v);

	Tprocessing sample;
	convert_from_texture_memory(sampleMem, sample);

	return sample;
}

template<typename T, typename Tmemory, typename Tprocessing>
inline T TextureImage<T, Tmemory, Tprocessing>::bilinear(uint32_t level, const vec2& uv) const
{
	float u = uv.x * m_mipPyramid[level].get_width()  - 0.5f;
	float v = uv.y * m_mipPyramid[level].get_height() - 0.5f;

	int32_t u0 = (int32_t)std::floor(u);
	int32_t v0 = (int32_t)std::floor(v);

	float du = u - u0;
	float dv = v - v0;

	return (1 - du) * ((1 - dv) * get_texel(level, u0    , v0) + dv * get_texel(level, u0    , v0 + 1)) +
	            du  * ((1 - dv) * get_texel(level, u0 + 1, v0) + dv * get_texel(level, u0 + 1, v0 + 1));
}

//-------------------------------------------//

template<typename T, typename Tmemory, typename Tprocessing>
Image<Tmemory> TextureImage<T, Tmemory, Tprocessing>::resize_to_power_of_2(Image<Tmemory> image, TextureRepeatMode repeatMode)
{
	//compute power-of-2 dims:
	//---------------
	uint32_t newWidth = 1;
	while(newWidth < image.get_width())
		newWidth *= 2;

	uint32_t newHeight = 1;
	while(newHeight < image.get_height())
		newHeight *= 2;

	//resample in x direction:
	//---------------
	auto resamplingWeightsX = compute_resampling_weights(image.get_width(), newWidth);
	std::unique_ptr<int32_t[]>& firstTexelX = resamplingWeightsX.first;
	std::unique_ptr<float[]>& weightsX = resamplingWeightsX.second;

	std::shared_ptr<Tmemory[]> resampledX = std::shared_ptr<Tmemory[]>(new Tmemory[newWidth * image.get_height()]);
	for(uint32_t y = 0; y < image.get_height(); y++)
	for(uint32_t x = 0; x < newWidth; x++)
	{
		Tprocessing val(0.0f);

		for(uint32_t i = 0; i < 4; i++)
		{
			int32_t sampleX = firstTexelX[x] + i;
			switch(repeatMode)
			{
			case TextureRepeatMode::REPEAT:
				sampleX %= image.get_width();
				break;
			case TextureRepeatMode::CLAMP_TO_EDGE:
				sampleX = std::min(std::max(sampleX, 0), (int32_t)image.get_width() - 1);
				break;
			}

			Tprocessing sample;
			convert_from_texture_memory(image.get(sampleX, y), sample);

			val = val + weightsX[x * 4 + i] * sample;
		}

		uint32_t idx = x + newWidth * y;
		Tmemory mem;
		convert_to_texture_memory(val, mem);
		resampledX[idx] = mem;
	}

	//resample in y direction:
	//---------------
	auto resamplingWeightsY = compute_resampling_weights(image.get_height(), newHeight);
	std::unique_ptr<int32_t[]>& firstTexelY = resamplingWeightsY.first;
	std::unique_ptr<float[]>& weightsY = resamplingWeightsY.second;	

	Image<Tmemory> resampledY(newWidth, newHeight, nullptr);
	for(uint32_t x = 0; x < newWidth; x++)
	for(uint32_t y = 0; y < newHeight; y++)
	{
		Tprocessing val(0.0f);

		for(uint32_t i = 0; i < 4; i++)
		{
			int32_t sampleY = firstTexelY[y] + i;
			switch(repeatMode)
			{
			case TextureRepeatMode::REPEAT:
				sampleY %= image.get_height();
				break;
			case TextureRepeatMode::CLAMP_TO_EDGE:
				sampleY = std::min(std::max(sampleY, 0), (int32_t)image.get_height() - 1);
				break;
			}

			uint32_t sampleIdx = x + newWidth * sampleY;
			Tprocessing sample;
			convert_from_texture_memory(resampledX[sampleIdx], sample);

			val = val + Tprocessing(weightsY[y * 4 + i]) * sample;
		}

		Tmemory mem;
		convert_to_texture_memory(val, mem);

		resampledY.put(x, y, mem);
	}

	//return:
	//---------------
	return resampledY;
}

template<typename T, typename Tmemory, typename Tprocessing>
std::pair<std::unique_ptr<int32_t[]>, std::unique_ptr<float[]>> TextureImage<T, Tmemory, Tprocessing>::compute_resampling_weights(uint32_t oldSize, uint32_t newSize)
{
	std::unique_ptr<int32_t[]> firstTexels = std::unique_ptr<int32_t[]>(new int32_t[newSize]);
	std::unique_ptr<float[]> weights = std::unique_ptr<float[]>(new float[newSize * 4]);

	for(uint32_t i = 0; i < newSize; i++)
	{
		float center = ((i + 0.5f) / (float)newSize) * (float)oldSize;
		int32_t firstTexel = (int32_t)std::floor((center - 2.0f) + 0.5f);
		firstTexels[i] = firstTexel;

		uint32_t iWeight = i * 4;
		weights[iWeight + 0] = lanczos_filter((firstTexel + 0.5f - center) / 2.0f);
		weights[iWeight + 1] = lanczos_filter((firstTexel + 1.5f - center) / 2.0f);
		weights[iWeight + 2] = lanczos_filter((firstTexel + 2.5f - center) / 2.0f);
		weights[iWeight + 3] = lanczos_filter((firstTexel + 3.5f - center) / 2.0f);

		float invSumWeight = 1.0f / (weights[iWeight + 0] + weights[iWeight + 1] + weights[iWeight + 2] + weights[iWeight + 3]);
		weights[iWeight + 0] *= invSumWeight;
		weights[iWeight + 1] *= invSumWeight;
		weights[iWeight + 2] *= invSumWeight;
		weights[iWeight + 3] *= invSumWeight;
	}

	return { std::move(firstTexels), std::move(weights) };
}

template<typename T, typename Tmemory, typename Tprocessing>
float TextureImage<T, Tmemory, Tprocessing>::lanczos_filter(float x)
{
    x = std::abs(x);
    if (x < FR_EPSILON) 
		return 1.0f;
    if (x > 1.0f) 
		return 0.0f;

    x *= FR_PI;
	return 2.0f * std::sin(x) * std::sin(x / 2.0f) / (x * x);
}

//-------------------------------------------//

inline void convert_from_texture_memory(uint32_t mem, vec4& val)
{
	val = vec4(
		(mem & 0xFF) / 255.0f,
		((mem >>  8) & 0xFF) / 255.0f,
		((mem >> 16) & 0xFF) / 255.0f,
		((mem >> 24) & 0xFF) / 255.0f
	);
}

inline void convert_to_texture_memory(vec4 val, uint32_t& mem)
{
	uint32_t r = (uint32_t)(val.r * 255.0f);
	uint32_t g = (uint32_t)(val.g * 255.0f);
	uint32_t b = (uint32_t)(val.b * 255.0f);
	uint32_t a = (uint32_t)(val.a * 255.0f);

	mem = r | (g << 8) | (b << 16) | (a << 24);
}

inline void convert_from_texture_memory(uint32_t mem, vec3& val)
{
	val = vec3(
		(mem & 0xFF) / 255.0f,
		((mem >>  8) & 0xFF) / 255.0f,
		((mem >> 16) & 0xFF) / 255.0f
	);
}

inline void convert_to_texture_memory(vec3 val, uint32_t& mem)
{
	uint32_t r = (uint32_t)(val.r * 255.0f);
	uint32_t g = (uint32_t)(val.g * 255.0f);
	uint32_t b = (uint32_t)(val.b * 255.0f);

	mem = r | (g << 8) | (b << 16);
}

inline void convert_from_texture_memory(uint8_t mem, float& val)
{
	val = (float)mem / 255.0f;
}

inline void convert_to_texture_memory(float val, uint8_t& mem)
{
	mem = (uint8_t)(val * 255.0f);
}

inline void convert_from_texture_memory(uint32_t mem, float& val)
{
	val = (mem >> 24) / 255.0f;
}

inline void convert_to_texture_memory(float val, uint32_t& mem)
{
	mem = ((uint32_t)(val * 255.0f)) << 24;
}

}; //namespace fr