/* brdf.hpp
 *
 * contains the definition of the brdf class, which represents
 * a bidrectional reflectance distribution function, used for sampling
 * ray directions
 */

#ifndef RURT_BRDF_H
#define RURT_BRDF_H

#include "intersection_info.hpp"

#include "quickmath.hpp"
using namespace qm;

//-------------------------------------------//

namespace rurt
{

class BRDF
{
public:
	BRDF() = default;

	virtual vec3 f(const IntersectionInfo& info, const vec3& i, const vec3& o, float& pdf) const = 0;
	virtual float pdf(const IntersectionInfo& info, const vec3& i, const vec3& o) const = 0;
};

} //namespace rurt

#endif //#ifndef RURT_BRDF_H