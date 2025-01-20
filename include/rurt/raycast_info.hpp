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

class Material;
class Light;

struct IntersectionInfo
{
	std::shared_ptr<const Material> material;
	std::shared_ptr<const Light> light;

	vec3 worldPos;
	vec3 objectPos;

	vec3 worldNormal;
	vec3 objectNormal;
	
	vec2 uv;

	struct Derivatives
	{
		vec3 dpdx;
		vec3 dpdy;
		vec2 duvdx;
		vec2 duvdy;
	} derivatives;
};

struct VisibilityTestInfo
{
	bool infinite;
	vec3 endPos;
};

}; //namespace rurt

#endif //#ifndef RURT_INTERSECTION_INFO_H