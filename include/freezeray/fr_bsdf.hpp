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
	BSDF(const IntersectionInfo& hitInfo, uint32_t numBxdfs, std::unique_ptr<const BXDF> bxdfs[FR_BSDF_MAX_COMPONENTS]);

	vec3 f(const vec3& wiWorld, const vec3& woWorld) const;
	vec3 sample_f(vec3& wiWorld, const vec3& woWorld, const vec3& u, float& pdf) const;
	float pdf(const vec3& wiWorld, const vec3& woWorld) const;

	bool is_delta() const;
	BXDFType get_type() const;

	vec3 world_to_local(const vec3& v) const;
	vec3 local_to_world(const vec3& v) const;

private:
	void world_to_local(const vec3& wiWorld, const vec3& woWorld, vec3& wi, vec3& wo) const;
	mat3 transform_between(const vec3& from, const vec3& to) const;

	vec3 m_normalShading;
	vec3 m_normalGeom;

	std::unique_ptr<const BXDF> m_bxdfs[FR_BSDF_MAX_COMPONENTS];
	uint32_t m_numBxdfs;
};

}

#endif