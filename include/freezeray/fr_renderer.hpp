/* fr_renderer.hpp
 *
 * contains the definition of the renderer class, which
 * is the central driver for the whole raytracer
 */

#ifndef FR_RENDERER_H
#define FR_RENDERER_H

#include <stdint.h>
#include <functional>
#include "fr_camera.hpp"
#include "fr_scene.hpp"
#include "fr_prng.hpp"

#include "quickmath.hpp"
using namespace qm;

//-------------------------------------------//

namespace fr
{

class Renderer
{
public:
	Renderer(const std::shared_ptr<const Camera>& cam, uint32_t imageW, uint32_t imageH);
	~Renderer();

	//renders the entire scene into the given image buffer, calling display occasionally to allow
	//the current result to be displayed
	virtual void render(const std::shared_ptr<const Scene>& scene,
	                    std::function<void(uint32_t x, uint32_t y, vec3 color)> writePixel, 
	                    std::function<void(float progress)> display, uint32_t displayFrequency = 1);

protected:
	virtual vec3 li(const std::shared_ptr<PRNG>& prng, const std::shared_ptr<const Scene>& scene, const Ray& ray) const = 0;

	vec3 sample_one_light(const std::shared_ptr<PRNG>& prng, const std::shared_ptr<const Scene>& scene, const IntersectionInfo& hitInfo, const vec3& wo) const;
	vec3 sample_one_light_mis(const std::shared_ptr<PRNG>& prng, const std::shared_ptr<const Scene>& scene, const IntersectionInfo& hitInfo, const vec3& wo) const;
	bool trace_visibility_ray(const std::shared_ptr<const Scene>& scene, const VisibilityTestInfo& visInfo) const;

	static float mis_power_heuristic(uint32_t nf, float pdff, uint32_t ng, float pdfg);

protected:
	std::shared_ptr<const Camera> m_cam;
	mat4 m_camInvView;
	mat4 m_camInvProj;

	uint32_t m_imageW;
	uint32_t m_imageH;

	Ray get_camera_ray(uint32_t x, uint32_t y) const;
	Ray get_camera_ray(vec2 uv) const;
};

}; //namespace fr

#endif //#ifndef FR_RENDERER_H