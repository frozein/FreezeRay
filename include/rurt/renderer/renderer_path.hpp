/* renderer_path.hpp
 *
 * contains the definition of a renderer that utilizes
 * path tracing for integration
 */

#ifndef RURT_RENDERER_PATH_H
#define RURT_RENDERER_PATH_H

#include "rurt/renderer.hpp"

//-------------------------------------------//

namespace rurt
{

class RendererPath : public Renderer
{
public:
	RendererPath(std::shared_ptr<const Camera> cam, uint32_t imageW, uint32_t imageH, uint32_t maxDepth, uint32_t samplesPerPixel, bool importanceSampling);
	~RendererPath();

private:
	uint32_t m_maxDepth;
	uint32_t m_samplesPerPixel;
	bool m_importanceSampling;

	vec3 li(const std::shared_ptr<const Scene>& scene, const Ray& ray) override;
	vec3 trace_path(const std::shared_ptr<const Scene>& scene, const Ray& ray);
};

}; //namespace rurt

#endif //#ifndef RURT_RENDERER_PATH_H