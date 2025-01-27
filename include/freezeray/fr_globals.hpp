/* fr_globals.hpp
 *
 * contains definitions for various frequently used constants and functions, such as pi
 */

#ifndef FR_GLOBALS_H
#define FR_GLOBALS_H

#include "quickmath.hpp"
using namespace qm;

//-------------------------------------------//

#define FR_EPSILON 0.001f

#define FR_PI 3.14159265358979323846f
#define FR_2_PI 6.28318530717958647693f

#define FR_INV_PI 0.31830988618379067154f
#define FR_INV_2_PI 0.15915494309189533577f

#define FR_GAMMA 2.2f
#define FR_INV_GAMMA 0.45454545454545454545f

//canonical up direction for local space calculations
#define FR_UP_DIR vec3(0.0f, 1.0f, 0.0f)
#define FR_DOWN_DIR vec3(0.0f, -1.0f, 0.0f)

//-------------------------------------------//

namespace fr
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
	return (sinTheta == 0.0f) ? 0.0f : std::min(std::max(w.z / sinTheta, -1.0f), 1.0f);
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

inline vec3 srgb_to_linear(const vec3& color)
{
	return vec3(
		std::powf(color.r, FR_GAMMA),
		std::powf(color.g, FR_GAMMA),
		std::powf(color.b, FR_GAMMA)
	);
}

inline vec3 linear_to_srgb(const vec3& color)
{
	return vec3(
		std::powf(color.r, FR_INV_GAMMA),
		std::powf(color.g, FR_INV_GAMMA),
		std::powf(color.b, FR_INV_GAMMA)
	);
}

}; //namespace fr

#endif //#ifndef FR_GLOBALS_H