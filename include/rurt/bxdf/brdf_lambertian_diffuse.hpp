/* brdf_lambertian_diffuse.hpp
 *
 * contains a definition for a lambertian diffuse BRDF model
 */

#ifndef RURT_BRDF_LAMBERTIAN_DIFFUSE
#define RURT_BRDF_LAMBERTIAN_DIFFUSE

#include "../bxdf.hpp"

//-------------------------------------------//

namespace rurt
{

class BRDFLambertianDiffuse : public BXDF
{
public:
	BRDFLambertianDiffuse(const vec3& color);

	vec3 f(const HitInfo& info, const vec3& wi, const vec3& wo) const override;
	vec3 sample_f(const HitInfo& info, vec3& wi, const vec3& wo, float& pdf) const override;
	float pdf(const HitInfo& info, const vec3& wi, const vec3& wo) const override;

	bool is_delta() const override { return false; }

private:
	vec3 m_color;
};

}; //namespace rurt

#endif //#ifndef RURT_BRDF_LAMBERTIAN_DIFFUSE