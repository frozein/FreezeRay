/* brdf_specular_reflection.hpp
 *
 * contains a definition for a specular BRDF model
 */

#ifndef RURT_BRDF_SPECULAR_REFLECTION_H
#define RURT_BRDF_SPECULAR_REFLECTION_H

#include "brdf.hpp"

//-------------------------------------------//

namespace rurt
{

class BRDFSpecularReflection : public BRDF
{
public:
	BRDFSpecularReflection(const vec3& color);

	vec3 f(const HitInfo& info, const vec3& wi, const vec3& wo) const override;
	vec3 sample_f(const HitInfo& info, vec3& wi, const vec3& wo, float& pdf) const override;
	float pdf(const HitInfo& info, const vec3& wi, const vec3& wo) const override;

	bool is_delta() const override { return true; }

private:
	vec3 m_color;
};

};

#endif //#ifndef RURT_BRDF_SPECULAR_REFLECTION_H