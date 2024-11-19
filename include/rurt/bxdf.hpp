/* bxdf.hpp
 *
 * contains the definition of the bxdf class, which represents
 * a bidrectional reflectance/transmittion/etc distribution function, 
 * used for sampling ray directions
 */

#ifndef RURT_BXDF_H
#define RURT_BXDF_H

#include "raycast_info.hpp"

#include "quickmath.hpp"
using namespace qm;

//-------------------------------------------//

namespace rurt
{

enum class BXDFType : uint32_t
{
	REFLECTION = 0,
	TRANSMISSION
};

class BXDF
{
public:
	BXDF(BXDFType type) : m_type(type) {};

	// wi and wo are relative to RURT_BRDF_UP_DIR
	virtual vec3 f(const HitInfo& info, const vec3& wi, const vec3& wo) const = 0;
	virtual vec3 sample_f(const HitInfo& info, vec3& wi, const vec3& wo, float& pdf) const = 0;
	virtual float pdf(const HitInfo& info, const vec3& wi, const vec3& wo) const = 0;

	virtual bool is_delta() const = 0;

	BXDFType type() const { return m_type; }

private:
	BXDFType m_type;
};

} //namespace rurt

#endif //#ifndef RURT_BRXF_H