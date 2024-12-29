/* light.hpp
 *
 * contains the definition of the light class, which represents
 * a general light source
 */

#ifndef RURT_LIGHT_H
#define RURT_LIGHT_H

#include "quickmath.hpp"
using namespace qm;

#include "raycast_info.hpp"
#include "scene.hpp"

//-------------------------------------------//

namespace rurt
{

class Light
{
public:
	Light(bool delta) : m_delta(delta) {};

	virtual vec3 sample_li(const IntersectionInfo& hitInfo, const vec3& u, vec3& wiWorld, VisibilityTestInfo& vis, float& pdf) const = 0;
	virtual vec3 power() const = 0;

	virtual std::shared_ptr<const Mesh> get_mesh(mat4& transform) const { return nullptr; }
	virtual vec3 le(const IntersectionInfo& hitInfo, const vec3& w) const { return vec3(0.0f); }

	bool is_delta() const { return m_delta; }

private:
	bool m_delta;
};

} //namespace rurt

#endif //#ifndef RURT_LIGHT_H