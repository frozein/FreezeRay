#include "../texture/fr_texture_image.hpp"

#include "stb_image.h"
#include "../fr_globals.hpp"
#include <math.h>

//-------------------------------------------//

namespace fr
{

template<typename T, typename Tmemory>
TextureImage<T, Tmemory>::TextureImage(uint32_t width, uint32_t height, std::shared_ptr<const Tmemory[]> image, TextureRepeatMode repeatMode, T multiplier) : 
	m_repeatMode(repeatMode), m_multiplier(multiplier)
{
	//validate input:
	//---------------
	if(width == 0 || height == 0)
		throw std::invalid_argument("dimensions cannot be 0");

	//determine if sizes are power of 2, resize if not:
	//---------------
	bool isPowerOf2 = true;

	uint32_t widthTemp = width;
	while(widthTemp != 1)
	{
		isPowerOf2 = isPowerOf2 && !(widthTemp & 1);
		widthTemp >>= 1;
	}

	uint32_t heightTemp = height;
	while(heightTemp != 1)
	{
		isPowerOf2 = isPowerOf2 && !(heightTemp & 1);
		heightTemp >>= 1;
	}

	Image baseImage;
	if(!isPowerOf2)
		baseImage = resize_to_power_of_2(width, height, image, repeatMode);
	else
	{
		baseImage.width = width;
		baseImage.height = height;
		baseImage.image = image;
	}

	m_mipPyramid.push_back(baseImage);

	//construct mip pyramid:
	//---------------
	uint32_t maxRes = std::max(baseImage.width, baseImage.height);
	uint32_t numLevels = 1;
	while(maxRes != 1)
	{
		maxRes >>= 1;
		numLevels++;
	}

	for(uint32_t i = 1; i < numLevels; i++)
	{
		uint32_t levelWidth  = std::max(1u, m_mipPyramid[i - 1].width  / 2);
		uint32_t levelHeight = std::max(1u, m_mipPyramid[i - 1].height / 2);

		std::shared_ptr<Tmemory[]> levelImage = std::shared_ptr<Tmemory[]>(new Tmemory[levelWidth * levelHeight]);
		
		for(int32_t y = 0; y < (int32_t)levelHeight; y++)
		for(int32_t x = 0; x < (int32_t)levelWidth; x++)
		{
			T sample = (
				get_texel(i - 1, 2 * x, 2 * y    ) + get_texel(i - 1, 2 * x + 1, 2 * y    ) +
				get_texel(i - 1, 2 * x, 2 * y + 1) + get_texel(i - 1, 2 * x + 1, 2 * y + 1)
			) / 4.0f;

			convert_to_texture_memory(sample, levelImage[x + levelWidth * y]);

		}
		
		m_mipPyramid.push_back({ levelWidth, levelHeight, levelImage });
	}
}

template<typename T, typename Tmemory>
T TextureImage<T, Tmemory>::evaluate(const IntersectionInfo& hitInfo) const
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

template<typename T, typename Tmemory>
std::shared_ptr<TextureImage<T, Tmemory>> TextureImage<T, Tmemory>::from_file(const std::string& path, bool hdr, TextureRepeatMode repeatMode, T multiplier)
{
	static_assert(sizeof(Tmemory) <= 4, "cannot load an image with >4 components");

	//load image from stbi:
	//---------------
	stbi_set_flip_vertically_on_load(true);

	int width;
	int height;
	int numChannels;
	uint8_t* imageRaw;
	if(hdr)
	{
		imageRaw = (uint8_t*)stbi_loadf(path.c_str(), &width, &height, &numChannels, sizeof(Tmemory) / sizeof(float));
		if(!imageRaw)
			throw std::runtime_error("failed to load image from \"" + path + "\"");
	}
	else
	{
		imageRaw = stbi_load(path.c_str(), &width, &height, &numChannels, sizeof(Tmemory));
		if(!imageRaw)
			throw std::runtime_error("failed to load image from \"" + path + "\"");
	}

	std::shared_ptr<Tmemory[]> image = std::shared_ptr<Tmemory[]>((Tmemory*)imageRaw, [](const Tmemory* img){ stbi_image_free((void*)img); });

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

	Image sizedImage;
	if(!isPowerOf2)
		sizedImage = resize_to_power_of_2(width, height, image, repeatMode);
	else
	{
		sizedImage.width = width;
		sizedImage.height = height;
		sizedImage.image = image;
	}

	//return:
	//---------------
	return std::make_shared<TextureImage<T, Tmemory>>(sizedImage.width, sizedImage.height, sizedImage.image, repeatMode, multiplier);
}

template<typename T, typename Tmemory, typename Tother>
static std::shared_ptr<TextureImage<T, Tmemory>> from_texture_image(const std::shared_ptr<TextureImage<Tother, Tmemory>>& image, T multiplier)
{
	return std::make_shared<TextureImage<T, Tmemory>>(image->m_mipPyramid, image->m_repeatMode, multiplier);
}

//-------------------------------------------//

template<typename T, typename Tmemory>
TextureImage<T, Tmemory>::TextureImage(const std::vector<Image>& mipPyramid, TextureRepeatMode repeatMode) :
	m_mipPyramid(mipPyramid), m_repeatMode(repeatMode)
{
	
}

template<typename T, typename Tmemory>
inline T TextureImage<T, Tmemory>::get_texel(uint32_t level, int32_t u, int32_t v) const
{
	const Image& image = m_mipPyramid[level];

	switch(m_repeatMode)
	{
	case TextureRepeatMode::REPEAT:
		u = (u % image.width  + image.width ) % image.width;
		v = (v % image.height + image.height) % image.height;
		break;
	case TextureRepeatMode::CLAMP_TO_EDGE:
		u = std::min(std::max(u, 0), (int32_t)image.width  - 1);
		v = std::min(std::max(v, 0), (int32_t)image.height - 1);
		break;
	default:
		throw std::invalid_argument("invalid repeat mode");
	}

	T sample;
	convert_from_texture_memory(image.image[u + image.width * v], sample);

	return sample;
}

template<typename T, typename Tmemory>
inline T TextureImage<T, Tmemory>::bilinear(uint32_t level, const vec2& uv) const
{
	float u = uv.x * m_mipPyramid[level].width  - 0.5f;
	float v = uv.y * m_mipPyramid[level].height - 0.5f;

	int32_t u0 = (int32_t)std::floor(u);
	int32_t v0 = (int32_t)std::floor(v);

	float du = u - u0;
	float dv = v - v0;

	return (1 - du) * ((1 - dv) * get_texel(level, u0    , v0) + dv * get_texel(level, u0    , v0 + 1)) +
	            du  * ((1 - dv) * get_texel(level, u0 + 1, v0) + dv * get_texel(level, u0 + 1, v0 + 1));
}

//-------------------------------------------//

template<typename T, typename Tmemory>
TextureImage<T, Tmemory>::Image TextureImage<T, Tmemory>::resize_to_power_of_2(uint32_t width, uint32_t height, std::shared_ptr<const Tmemory[]> image, TextureRepeatMode repeatMode)
{
	//compute power-of-2 dims:
	//---------------
	uint32_t newWidth = 1;
	while(newWidth < width)
		newWidth *= 2;

	uint32_t newHeight = 1;
	while(newHeight < height)
		newHeight *= 2;

	//resample in x direction:
	//---------------
	auto resamplingWeightsX = compute_resampling_weights(width, newWidth);
	std::unique_ptr<int32_t[]>& firstTexelX = resamplingWeightsX.first;
	std::unique_ptr<float[]>& weightsX = resamplingWeightsX.second;

	std::shared_ptr<Tmemory[]> resampledX = std::shared_ptr<Tmemory[]>(new Tmemory[newWidth * height]);
	for(uint32_t y = 0; y < height; y++)
	for(uint32_t x = 0; x < newWidth; x++)
	{
		T val(0.0f);

		for(uint32_t i = 0; i < 4; i++)
		{
			int32_t sampleX = firstTexelX[x] + i;
			switch(repeatMode)
			{
			case TextureRepeatMode::REPEAT:
				sampleX %= width;
				break;
			case TextureRepeatMode::CLAMP_TO_EDGE:
				sampleX = std::min(std::max(sampleX, 0), (int32_t)width - 1);
				break;
			}

			uint32_t sampleIdx = sampleX + width * y;
			T sample;
			convert_from_texture_memory(image[sampleIdx], sample);

			val = val + weightsX[x * 4 + i] * sample;
		}

		uint32_t idx = x + newWidth * y;
		Tmemory mem;
		convert_to_texture_memory(val, mem);
		resampledX[idx] = mem;
	}

	//resample in y direction:
	//---------------
	auto resamplingWeightsY = compute_resampling_weights(height, newHeight);
	std::unique_ptr<int32_t[]>& firstTexelY = resamplingWeightsY.first;
	std::unique_ptr<float[]>& weightsY = resamplingWeightsY.second;	

	std::shared_ptr<Tmemory[]> resampledY = std::shared_ptr<Tmemory[]>(new Tmemory[newWidth * newHeight]);
	for(uint32_t x = 0; x < newWidth; x++)
	for(uint32_t y = 0; y < newHeight; y++)
	{
		T val(0.0f);

		for(uint32_t i = 0; i < 4; i++)
		{
			int32_t sampleY = firstTexelY[y] + i;
			switch(repeatMode)
			{
			case TextureRepeatMode::REPEAT:
				sampleY %= height;
				break;
			case TextureRepeatMode::CLAMP_TO_EDGE:
				sampleY = std::min(std::max(sampleY, 0), (int32_t)height - 1);
				break;
			}

			uint32_t sampleIdx = x + newWidth * sampleY;
			T sample;
			convert_from_texture_memory(resampledX[sampleIdx], sample);

			val = val + T(weightsY[y * 4 + i]) * sample;
		}

		uint32_t idx = x + newWidth * y;
		Tmemory mem;
		convert_to_texture_memory(val, mem);
		resampledY[idx] = mem;
	}

	//return:
	//---------------
	return { newWidth, newHeight, resampledY };
}

template<typename T, typename Tmemory>
std::pair<std::unique_ptr<int32_t[]>, std::unique_ptr<float[]>> TextureImage<T, Tmemory>::compute_resampling_weights(uint32_t oldSize, uint32_t newSize)
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

template<typename T, typename Tmemory>
float TextureImage<T, Tmemory>::lanczos_filter(float x)
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
	mem = (uint32_t)(val * 255.0f) << 24;
}

}; //namespace fr