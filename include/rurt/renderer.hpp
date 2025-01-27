/* renderer.hpp
 *
 * contains the definition of the renderer class, which
 * is the central driver for the whole raytracer
 */

#ifndef RURT_RENDERER_H
#define RURT_RENDERER_H

#include <stdint.h>
#include <functional>
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
	Renderer(const std::shared_ptr<const Camera>& cam, uint32_t imageW, uint32_t imageH);
	~Renderer();

	//renders the entire scene into the given image buffer, calling display occasionally to allow
	//the current result to be displayed
	void render(const std::shared_ptr<const Scene>& scene,
	            std::function<void(uint32_t x, uint32_t y, vec3 color)> writePixel, 
				std::function<void()> display);

protected:
	virtual vec3 li(const std::shared_ptr<const Scene>& scene, const Ray& ray) = 0;

	vec3 uniform_sample_one_light(const std::shared_ptr<const Scene>& scene, const IntersectionInfo& hitInfo, const vec3& wo) const;
	bool trace_visibility_ray(const std::shared_ptr<const Scene>& scene, const IntersectionInfo& initialHitInfo, const vec3& wi, const vec3& wo, const VisibilityTestInfo& visInfo) const;

	static vec3 random_dir_sphere();
	static vec3 random_dir_hemisphere(const vec3& normal);

private:
	std::shared_ptr<const Camera> m_cam;
	mat4 m_camInvView;
	mat4 m_camInvProj;

	uint32_t m_imageW;
	uint32_t m_imageH;

	Ray get_camera_ray(uint32_t x, uint32_t y) const;
};

}; //namespace rurt

#endif //#ifndef RURT_RENDERER_H