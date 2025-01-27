/* fr_bxdf.hpp
 *
 * contains the definition of the bxdf class, which represents
 * a bidrectional reflectance/transmittion/etc distribution function, 
 * used for sampling ray directions
 */

#ifndef FR_BXDF_H
#define FR_BXDF_H

#include "quickmath.hpp"
using namespace qm;

//-------------------------------------------//

namespace fr
{

enum class BXDFType : uint32_t
{
	REFLECTION   = (1 << 0),
	TRANSMISSION = (1 << 1),
	BOTH = REFLECTION | TRANSMISSION
};

class BXDF
{
public:
	BXDF(bool delta, BXDFType type) : m_delta(delta), m_type(type) {};

	// wi and wo are relative to FR_BRDF_UP_DIR
	virtual vec3 f(const vec3& wi, const vec3& wo) const = 0;
	virtual vec3 sample_f(vec3& wi, const vec3& wo, const vec2& u, float& pdf) const = 0;
	virtual float pdf(const vec3& wi, const vec3& wo) const = 0;

	bool is_delta() const { return m_delta; }
	BXDFType type() const { return m_type; }

private:
	bool m_delta;
	BXDFType m_type;
};

} //namespace fr

#endif //#ifndef FR_BRXF_H