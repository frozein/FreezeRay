/* fr_ray.hpp
 * 
 * contains the definition of the ray class, which
 * is just a helper to hold both an origin and direction
 */

#ifndef FR_RAY_H
#define FR_RAY_H

#include "quickmath.hpp"
using namespace qm; //TODO: do we want to have this?

//-------------------------------------------//

namespace fr
{

class Ray
{
public:
	Ray() : m_orig(0.0f), m_dir(0.0f) {};
	Ray(const vec3& origin, const vec3& direction) : m_orig(origin), m_dir(direction), m_hasDifferentials(false) {}; 
	Ray(const Ray& mainRay, const Ray& differentialX, const Ray& differentialY) : 
		m_orig(mainRay.m_orig), m_dir(mainRay.m_dir), m_hasDifferentials(true), m_diffOrigX(differentialX.m_orig), 
		m_diffOrigY(differentialY.m_orig), m_diffDirX(differentialX.m_dir), m_diffDirY(differentialY.m_dir) {}; 

	inline const vec3& origin() const { return m_orig; }
	inline const vec3& direction() const { return m_dir; }

	inline vec3 at(float t) const { return m_orig + t * m_dir; }

	inline Ray transformed(mat4 transform, mat4 transformNoTranslate) const 
	{
		if(m_hasDifferentials)
		{
			return Ray(transformed(m_orig, m_dir, transform, transformNoTranslate),
			           transformed(m_diffOrigX, m_diffDirX, transform, transformNoTranslate),
			           transformed(m_diffOrigY, m_diffDirY, transform, transformNoTranslate));
		}
		else
			return transformed(m_orig, m_dir, transform, transformNoTranslate);
	};

	inline bool has_differentials() const { return m_hasDifferentials; }
	inline Ray differential_x() const { return Ray(m_diffOrigX, m_diffDirX); }
	inline Ray differential_y() const { return Ray(m_diffOrigY, m_diffDirY); }

private:
	vec3 m_orig;
	vec3 m_dir;

	bool m_hasDifferentials;
	vec3 m_diffOrigX;
	vec3 m_diffOrigY;
	vec3 m_diffDirX;
	vec3 m_diffDirY;

	static inline Ray transformed(vec3 orig, vec3 dir, mat4 transform, mat4 transformNoTranslate) 
	{ 
		return Ray((transform * vec4(orig, 1.0)).xyz(), (transformNoTranslate * vec4(dir, 1.0)).xyz()); 
	};
};

}; //namespace fr

#endif //#ifndef FR_RAY_H