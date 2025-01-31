/* fr_light.hpp
 *
 * contains the definition of the light class, which represents
 * a general light source
 */

#ifndef FR_LIGHT_H
#define FR_LIGHT_H

#include "quickmath.hpp"
using namespace qm;

#include "fr_raycast_info.hpp"
#include "fr_scene.hpp"

//-------------------------------------------//

namespace fr
{

class Light
{
public:
	Light(bool delta, bool infinite) : m_delta(delta), m_infinite(infinite) {};

	virtual vec3 sample_li(const IntersectionInfo& hitInfo, const vec3& u, vec3& wiWorld, VisibilityTestInfo& vis, float& pdf) const = 0;
	virtual vec3 power() const = 0;

	virtual std::shared_ptr<const Mesh> get_mesh(mat4& transform) const { return nullptr; }
	virtual vec3 le(const IntersectionInfo& hitInfo, const vec3& w) const { return vec3(0.0f); }

	bool is_delta() const { return m_delta; }
	bool is_infinite() const { return m_infinite; }

private:
	bool m_delta;
	bool m_infinite;
};

} //namespace fr

#endif //#ifndef FR_LIGHT_H