/* material.hpp
 *
 * contains the definition of the material class, which represents
 * aspects of a particular type of surface, as well as info on how rays
 * should interact with it
 */

#ifndef RURT_MATERIAL_H
#define RURT_MATERIAL_H

#include <string>
#include <memory>

#include "bxdf.hpp"
#include "raycast_info.hpp"

//-------------------------------------------//

namespace rurt
{

class Material
{
public:
	Material(const std::string& name, bool delta, BXDFType type);

	const std::string& get_name() const;
	void set_name(const std::string& name);

	virtual vec3 bsdf_f(const IntersectionInfo& hitInfo, const vec3& wiWorld, const vec3& woWorld) const = 0;
	virtual vec3 bsdf_sample_f(const IntersectionInfo& hitInfo, vec3& wiWorld, const vec3& woWorld, const vec2& u, float& pdf) const = 0;
	virtual float bsdf_pdf(const IntersectionInfo& hitInfo, const vec3& wiWorld, const vec3& woWorld) const = 0;

	bool bsdf_is_delta() const;
	BXDFType bsdf_type() const;

private:
	std::string m_name;
	BXDFType m_type;
	bool m_delta;

protected:
	static vec3 world_to_local(const vec3& normal, const vec3& v);
	static void world_to_local(const vec3& normal, const vec3& wiWorld, const vec3& woWorld, vec3& wi, vec3& wo);
	static vec3 local_to_world(const vec3& normal, const vec3& v);
	static mat3 transform_between(const vec3& from, const vec3& to);
};

}; //namespace rurt

#endif //#ifndef RURT_MATERIAL_H