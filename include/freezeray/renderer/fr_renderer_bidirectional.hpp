/* fr_renderer_bidirectional.hpp
 *
 * contains the definition of a renderer that utilizes
 * bidirectional path tracing for integration
 */

#ifndef FR_RENDERER_BIDIRECTIONAL_H
#define FR_RENDERER_BIDIRECTIONAL_H

#include "../fr_renderer.hpp"

//-------------------------------------------//

namespace fr
{

class RendererBidirectional : public Renderer
{
public:
	RendererBidirectional(std::shared_ptr<const Camera> cam, uint32_t imageW, uint32_t imageH, uint32_t maxDepth, 
	                      uint32_t samplesPerPixel, bool importanceSampling, bool multipleImportanceSampling);
	~RendererBidirectional();

private:
	struct PathVertex
	{
		enum class Type
		{
			SURFACE,
			LIGHT,
			CAMERA,

			NONE
		} type;

		vec3 mult;

		IntersectionInfo intersection;

		static PathVertex from_surface(const IntersectionInfo& surface, const vec3& mult);
		static PathVertex from_light(std::shared_ptr<const Light> light, const IntersectionInfo& hitInfo, const vec3& mult);
		static PathVertex from_camera(std::shared_ptr<const Camera> camera, const Ray& ray, const vec3& mult);
	};

	uint32_t m_maxDepth;
	uint32_t m_samplesPerPixel;
	bool m_importanceSampling;
	bool m_mis;

	vec3 li(const std::shared_ptr<const Scene>& scene, const Ray& ray) const override;

	vec3 trace_bidirectional_path(const std::shared_ptr<const Scene>& scene, const Ray& ray) const;
	void trace_walk(const std::shared_ptr<const Scene>& scene, const Ray& ray, vec3 mult, float pdf, std::vector<PathVertex>& vertices) const;
};

}; //namespace fr
 
 #endif //#ifndef FR_RENDERER_BIDIRECTIONAL_H