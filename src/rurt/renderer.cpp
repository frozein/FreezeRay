#include "renderer.hpp"

//-------------------------------------------//

namespace rurt
{

Renderer::Renderer(uint32_t imageW, uint32_t imageH)
{
	this->imageW = imageW;
	this->imageH = imageH;
}

Renderer::~Renderer()
{

}

bool Renderer::draw_scanline(uint32_t y, uint32_t* buf)
{
	float val = (float)y / (float)imageH;

	for(uint32_t x = 0; x < imageW; x++)
	{
		uint8_t r = (uint8_t)(val * 255.0f);
		uint8_t g = (uint8_t)(val * 255.0f);
		uint8_t b = (uint8_t)(val * 255.0f);

		uint32_t writeColor = r | ((uint32_t)g >> 8) | ((uint32_t)b >> 16);
		buf[x] = writeColor;
	}

	return true;
}

}; //namespace rurt