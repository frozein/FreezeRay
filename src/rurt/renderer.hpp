/* renderer.hpp
 *
 * contains the definition of the renderer class, which
 * is the central driver for the whole raytracer
 */

#ifndef RURT_RENDERER_H
#define RURT_RENDERER_H

#include <stdint.h>

//-------------------------------------------//

namespace rurt
{

class Renderer
{
public:
	Renderer(uint32_t imageW, uint32_t imageH);
	~Renderer();

	bool draw_scanline(uint32_t y, uint32_t* buf);

private:
	uint32_t imageW;
	uint32_t imageH;
};

}; //namespace rurt

#endif //#ifndef RURT_RENDERER_H