/* ray.hpp
 * 
 * contains the definition of the ray class, which
 * is just a helper to hold both an origin and direction
 */

#ifndef RURT_RAY_H
#define RURT_RAY_H

#include "quickmath.hpp"
using namespace qm; //TODO: do we want to have this?

//-------------------------------------------//

namespace rurt
{

class Ray
{
public:
	Ray() : m_orig(0.0f), m_dir(0.0f) {};
	Ray(const vec3& origin, const vec3& direction) : m_orig(origin), m_dir(direction) {}; 

	inline const vec3& origin() const { return m_orig; }
	inline const vec3& direction() const { return m_dir; }

	inline vec3 at(float t) const { return m_orig + t * m_dir; }

	inline Ray transformed(mat4 transform, mat4 transformNoTranslate) const 
	{ 
		return Ray((transform * vec4(m_orig, 1.0)).xyz(), (transformNoTranslate * vec4(m_dir, 1.0)).xyz()); 
	};

private:
	vec3 m_orig;
	vec3 m_dir;
};

}; //namespace rurt

#endif //#ifndef RURT_RAY_H