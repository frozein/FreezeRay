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
		uint8_t a = UINT8_MAX; //each pixel fully opaque

		uint32_t writeColor = ((uint32_t)r << 24) | ((uint32_t)g << 16) | ((uint32_t)b << 8) | (uint32_t)a;
		buf[x] = writeColor;
	}

	return true;
}

}; //namespace rurt