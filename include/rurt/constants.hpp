/* constants.hpp
 *
 * contains definitions for various frequently used constants, such as pi
 */

#ifndef RURT_CONSTANTS_H
#define RURT_CONSTANTS_H

#define RURT_EPSILON 0.001f

#define RURT_PI 3.14159265358979323846f
#define RURT_2_PI 6.28318530717958647693f

#define RURT_INV_PI 0.31830988618379067154f
#define RURT_INV_2_PI 0.15915494309189533577f

#define RURT_GAMMA 2.2f
#define RURT_INV_GAMMA 0.45454545454545454545f

//canonical up direction for local space calculations
#define RURT_UP_DIR vec3(0.0f, 1.0f, 0.0f)

//-------------------------------------------//

namespace rurt
{

//TRIG FUNCTIONS FOR RAYS IN LOCAL SPACE:
//-------------------------------------------//

inline float cos_theta(const vec3& w)
{
	return w.y;
}

inline float cos_theta2(const vec3& w)
{
	return w.y * w.y;
}

inline float sin_theta2(const vec3& w)
{
	return std::max(0.0f, 1.0f - w.y * w.y);
}

inline float sin_theta(const vec3& w)
{
	return std::sqrtf(sin_theta2(w));
}

inline float tan_theta(const vec3& w)
{
	return sin_theta(w) / cos_theta(w);
}

inline float tan_theta2(const vec3& w)
{
	return sin_theta2(w) / cos_theta2(w);
}

inline float cos_phi(const vec3& w)
{
	float sinTheta = sin_theta(w);
	return (sinTheta == 0.0f) ? 1.0f : std::min(std::max(w.x / sinTheta, -1.0f), 1.0f);
}

inline float sin_phi(const vec3& w)
{
	float sinTheta = sin_theta(w);
	return (sinTheta == 0.0f) ? 1.0f : std::min(std::max(w.z / sinTheta, -1.0f), 1.0f);
}

inline float cos_phi2(const vec3& w)
{
	return cos_phi(w) * cos_phi(w);
}

inline float sin_phi2(const vec3& w)
{
	return sin_phi(w) * sin_phi(w);
}

inline bool same_hemisphere(const vec3& w1, const vec3& w2)
{
	return w1.y * w2.y > 0.0f;
}

//COLOR HELPERS:
//-------------------------------------------//

//TODO

}; //namespace rurt

#endif //#ifndef RURT_CONSTANTS_H