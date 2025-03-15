/* fr_bsdf.hpp
 *
 * contains the definition of the BSDF class, which represents a
 * collection of 1 or more BXDFs
 */

#ifndef FR_BSDF_H
#define FR_BDSF_H

#include "fr_bxdf.hpp"
#include "fr_raycast_info.hpp"

//-------------------------------------------//

namespace fr
{

#define FR_BSDF_MAX_COMPONENTS 8

//-------------------------------------------//

class BSDF
{
public:
	BSDF(const vec3& hitNormal);

	vec3 f(const vec3& wiWorld, const vec3& woWorld, BXDFflags flags) const;
	vec3 sample_f(vec3& wiWorld, const vec3& woWorld, const vec3& u, float& pdf, BXDFflags flags, BXDFflags& sampledFlags) const;
	float pdf(const vec3& wiWorld, const vec3& woWorld, BXDFflags flags) const;

	BXDFflags get_flags() const;

	void add_bxdf(std::shared_ptr<const BXDF> bxdf, vec3 color);

private:
	mat3 m_localToWorld;
	mat3 m_worldToLocal;

	std::shared_ptr<const BXDF> m_bxdfs[FR_BSDF_MAX_COMPONENTS];
	vec3 m_colors[FR_BSDF_MAX_COMPONENTS];
	uint32_t m_numBxdfs;

	mat3 transform_between(const vec3& from, const vec3& to) const;
};

}

#endif