/* fr_light_area.hpp
 *
 * contains a definition for an area light source
 */

#ifndef FR_LIGHT_AREA_H
#define FR_LIGHT_AREA_H

#include "../fr_light.hpp"
#include "../fr_mesh.hpp"
#include "../fr_scene.hpp"
#include "../fr_distribution.hpp"

//-------------------------------------------//

namespace fr
{

//by default, the area light emits light uniformly on every point on the mesh
//override this class if you want more specific behavior
class LightArea : public Light
{
public:
	LightArea(const std::shared_ptr<const Mesh>& mesh, const mat4& transform, const vec3& intensity);

	virtual vec3 sample_li(const IntersectionInfo& hitInfo, const vec3& u, vec3& wiWorld, VisibilityTestInfo& vis, float& pdf) const override;
	virtual float pdf_li(const IntersectionInfo& hitInfo, const vec3& w) const override;
	virtual vec3 power() const override;

	virtual std::shared_ptr<const Mesh> get_mesh(mat4& transform) const override;
	virtual vec3 le(const IntersectionInfo& hitInfo, const vec3& w) const override;

private:
	std::shared_ptr<const Mesh> m_mesh;
	mat4 m_transform;
	vec3 m_intensity;

	float m_area;
	std::unique_ptr<Distribution<uint32_t>> m_triDistribution;

	vec3 sample_mesh_area(const vec3& u) const;
};

}; //namespace fr

#endif //#ifndef FR_LIGHT_AREA_H