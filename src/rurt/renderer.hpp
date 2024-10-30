/* renderer.hpp
 *
 * contains the definition of the renderer class, which
 * is the central driver for the whole raytracer
 */

#ifndef RURT_RENDERER_H
#define RURT_RENDERER_H

#include <stdint.h>
#include "camera.hpp"
#include "scene.hpp"

#include "quickmath.hpp"
using namespace qm;

//-------------------------------------------//

namespace rurt
{

class Renderer
{
public:
	Renderer(std::shared_ptr<const Scene> scene, std::shared_ptr<const Camera> cam, uint32_t imageW, uint32_t imageH);
	~Renderer();

	void draw_scanline(uint32_t y, uint32_t* buf);

private:
	std::shared_ptr<const Scene> m_scene;

	std::shared_ptr<const Camera> m_cam;
	mat4 m_camInvView;
	mat4 m_camInvProj;

	uint32_t m_imageW;
	uint32_t m_imageH;
};

}; //namespace rurt

#endif //#ifndef RURT_RENDERER_H