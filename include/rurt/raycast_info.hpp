/* raycast_info.hpp
 * 
 * contains the defintion of the intersection info struct,
 * which includes information about a particular ray intersection
 */

#ifndef RURT_INTERSECTION_INFO_H
#define RURT_INTERSECTION_INFO_H

#include "quickmath.hpp"
using namespace qm;

//-------------------------------------------//

namespace rurt
{

struct HitInfo
{
	vec3 worldPos;
	vec3 objectPos;

	vec3 worldNormal;
	vec3 objectNormal;
	
	vec2 uv;
};

struct MissInfo
{
	vec3 skyColor;
	vec3 skyEmission;
};

union RaycastInfo
{
	HitInfo hitInfo;
	MissInfo missInfo;
};

struct VisibilityTestInfo
{
	bool infinite;
	vec3 endPos;
};

}; //namespace rurt

#endif //#ifndef RURT_INTERSECTION_INFO_H