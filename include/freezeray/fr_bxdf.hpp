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

enum class BXDFflags : uint32_t
{
	REFLECTION   = (1 << 0),
	TRANSMISSION = (1 << 1),
	DELTA        = (1 << 2),

	NONE         = 0,
	ALL          = UINT32_MAX
};

class BXDF
{
public:
	BXDF(BXDFflags flags) : m_flags(flags) {};

	// wi and wo are relative to FR_BRDF_UP_DIR
	virtual vec3 f(const vec3& wi, const vec3& wo) const = 0;
	virtual vec3 sample_f(vec3& wi, const vec3& wo, const vec2& u, float& pdf) const = 0;
	virtual float pdf(const vec3& wi, const vec3& wo) const = 0;

	BXDFflags get_flags() const { return m_flags; }

private:
	bool m_delta;
	BXDFflags m_flags;
};

//-------------------------------------------//

inline BXDFflags operator|(BXDFflags l, BXDFflags r)
{
	using T = std::underlying_type_t<BXDFflags>;
	return static_cast<BXDFflags>(static_cast<T>(l) | static_cast<T>(r));
}

inline BXDFflags operator&(BXDFflags l, BXDFflags r)
{
	using T = std::underlying_type_t<BXDFflags>;
	return static_cast<BXDFflags>(static_cast<T>(l) & static_cast<T>(r));
}

inline BXDFflags operator~(BXDFflags l)
{
	using T = std::underlying_type_t<BXDFflags>;
	return static_cast<BXDFflags>(~static_cast<T>(l));
}

} //namespace fr

#endif //#ifndef FR_BRXF_H