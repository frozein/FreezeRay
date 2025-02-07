/* fr_renderer_path.hpp
 *
 * contains the definition of a renderer that utilizes
 * path tracing for integration
 */

#ifndef FR_RENDERER_PATH_H
#define FR_RENDERER_PATH_H

#include "../fr_renderer.hpp"

//-------------------------------------------//

namespace fr
{

class RendererPath : public Renderer
{
public:
	RendererPath(std::shared_ptr<const Camera> cam, uint32_t imageW, uint32_t imageH, uint32_t maxDepth, 
		         uint32_t samplesPerPixel, bool importanceSampling, bool multipleImportanceSampling);
	~RendererPath();

private:
	uint32_t m_maxDepth;
	uint32_t m_samplesPerPixel;
	bool m_importanceSampling;
	bool m_mis;

	vec3 li(const std::shared_ptr<const Scene>& scene, const Ray& ray) override;
	vec3 trace_path(const std::shared_ptr<const Scene>& scene, const Ray& ray);
};

}; //namespace fr

#endif //#ifndef FR_RENDERER_PATH_H