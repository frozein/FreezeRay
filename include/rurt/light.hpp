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

//-------------------------------------------//

namespace rurt
{

class Light
{
public:
	Light(bool delta) : m_delta(delta) {};

	virtual vec3 sample_li(const HitInfo& hitInfo, const vec2& u, vec3& wiWorld, VisibilityTestInfo& vis, float& pdf) const = 0;
	virtual vec3 power() const = 0;

	bool is_delta() const { return m_delta; }

private:
	bool m_delta;
};

} //namespace rurt

#endif //#ifndef RURT_LIGHT_H