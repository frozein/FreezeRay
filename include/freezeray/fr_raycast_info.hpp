/* fr_raycast_info.hpp
 * 
 * contains the defintion of the intersection info struct,
 * which includes information about a particular ray intersection
 */

#ifndef FR_INTERSECTION_INFO_H
#define FR_INTERSECTION_INFO_H

#include "quickmath.hpp"
using namespace qm;

//-------------------------------------------//

namespace fr
{

class BSDF;
class Light;
class Camera;

struct IntersectionInfo
{
	std::shared_ptr<const BSDF> bsdf;
	std::shared_ptr<const Light> light;
	std::shared_ptr<const Camera> camera; //only used for bidirectional pt

	vec3 wo;
	vec3 pos;
	
	vec3 shadingNormal;
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

}; //namespace fr

#endif //#ifndef FR_INTERSECTION_INFO_H