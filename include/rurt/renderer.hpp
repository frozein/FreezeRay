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
	Renderer(std::shared_ptr<const Scene> scene, std::shared_ptr<const Camera> cam, uint32_t imageW, uint32_t imageH, uint32_t spp);
	~Renderer();

	void draw_scanline(uint32_t y, uint32_t* buf);

private:
	std::shared_ptr<const Scene> m_scene;

	std::shared_ptr<const Camera> m_cam;
	mat4 m_camInvView;
	mat4 m_camInvProj;

	uint32_t m_imageW;
	uint32_t m_imageH;
	uint32_t m_spp;

	vec3 trace_path(const Ray& cameraRay);

	Ray get_camera_ray(uint32_t x, uint32_t y) const;
	static vec3 random_dir_hemisphere();
	static mat3 transform_between(const vec3& from, const vec3& to);
};

}; //namespace rurt

#endif //#ifndef RURT_RENDERER_H