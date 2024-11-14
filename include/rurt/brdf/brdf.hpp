/* brdf.hpp
 *
 * contains the definition of the brdf class, which represents
 * a bidrectional reflectance distribution function, used for sampling
 * ray directions
 */

#ifndef RURT_BRDF_H
#define RURT_BRDF_H

#include "../raycast_info.hpp"

#include "../quickmath.hpp"
using namespace qm;

//-------------------------------------------//

namespace rurt
{

class BRDF
{
public:
	BRDF() = default;

	// wi and wo are relative to RURT_BRDF_UP_DIR
	virtual vec3 f(const HitInfo& info, const vec3& wi, const vec3& wo) const = 0;
	virtual float pdf(const HitInfo& info, const vec3& wi, const vec3& wo) const = 0;
};

} //namespace rurt

#endif //#ifndef RURT_BRDF_H