/* fr_image.hpp
 *
 * contains a definition for an image, stored with cache locality
 * in mind
 */

#ifndef FR_IMAGE_H
#define FR_IMAGE_H

#include <memory>

//-------------------------------------------//

#define FR_IMAGE_BLOCK_SIZE 8

//-------------------------------------------//

namespace fr
{

template<typename T>
class Image
{
public:
	Image(uint32_t width, uint32_t height, std::shared_ptr<const T[]> mem);

	T get(uint32_t x, uint32_t y) const;
	void put(uint32_t x, uint32_t y, T val);

private:
	uint32_t m_width;
	uint32_t m_height;
	uint32_t m_widthBlocks;
	uint32_t m_heightBlocks;
	std::shared_ptr<std::shared_ptr<T[]>> m_blocks;
};

//-------------------------------------------//

template<typename T>
Image<T>::Image(uint32_t width, uint32_t height, std::shared_ptr<const T[]> mem)
    : m_width(width), m_height(height)
    , m_widthBlocks((width + FR_IMAGE_BLOCK_SIZE - 1) / FR_IMAGE_BLOCK_SIZE), m_heightBlocks((height + FR_IMAGE_BLOCK_SIZE - 1) / FR_IMAGE_BLOCK_SIZE)
{
	m_blocks = std::make_shared<std::shared_ptr<T[]>>(m_widthBlocks * m_heightBlocks);
    
    for(uint32_t y = 0; y < m_widthBlocks ; y++)
	for(uint32_t x = 0; x < m_heightBlocks; x++)
		m_blocks[by * m_blocksX + bx] = std::make_shared<T[]>(FR_IMAGE_BLOCK_SIZE * FR_IMAGE_BLOCK_SIZE);
    
    if(mem) 
	{
        for(uint32_t y = 0; y < height; y++)
		for(uint32_t x = 0; x < width ; x++)
			put(x, y, mem[x + width * y]);
    }
}

template<typename T>
T Image<T>::get(uint32_t x, uint32_t y) const
{
    uint32_t blockX = x / FR_IMAGE_BLOCK_SIZE;
	uint32_t blockY = y / FR_IMAGE_BLOCK_SIZE;
	uint32_t localX = x % FR_IMAGE_BLOCK_SIZE;
	uint32_t localY = y % FR_IMAGE_BLOCK_SIZE;
    
    const auto& block = m_blocks[blockX + m_widthBlocks * blockY];
    return block[localX + FR_IMAGE_BLOCK_SIZE * localY];
}

template<typename T>
void Image<T>::put(uint32_t x, uint32_t y, T val)
{
    uint32_t blockX = x / FR_IMAGE_BLOCK_SIZE;
	uint32_t blockY = y / FR_IMAGE_BLOCK_SIZE;
	uint32_t localX = x % FR_IMAGE_BLOCK_SIZE;
	uint32_t localY = y % FR_IMAGE_BLOCK_SIZE;
    
    auto& block = m_blocks[blockX + m_widthBlocks * blockY];
    block[localX + FR_IMAGE_BLOCK_SIZE * localY] = val;
}


}; //namespace fr

#endif //#ifndef FR_IMAGE_H