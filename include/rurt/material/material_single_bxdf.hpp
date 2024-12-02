/* material_single_bxdf.hpp
 *
 * contains a definition for a material consisting of a single BXDF
 */

#ifndef RURT_MATERIAL_SINGLE_BXDF_H
#define RURT_MATERIAL_SINGLE_BXDF_H

#include "../material.hpp"

//-------------------------------------------//

namespace rurt
{

class MaterialSingleBXDF : public Material
{
public:
	MaterialSingleBXDF(const std::string& name, std::shared_ptr<BXDF> bxdf, const vec3& color);

	vec3 bsdf_f(const HitInfo& hitInfo, const vec3& wiWorld, const vec3& woWorld) const override;
	vec3 bsdf_sample_f(const HitInfo& hitInfo, vec3& wiWorld, const vec3& woWorld, const vec2& u, float& pdf) const override;
	float bsdf_pdf(const HitInfo& hitInfo, const vec3& wiWorld, const vec3& woWorld) const override;

private:
	std::shared_ptr<BXDF> m_bxdf;
	vec3 m_color;
};

}; //namespace rurt

#endif //#ifndef RURT_MATERIAL_SINGLE_BXDF_H