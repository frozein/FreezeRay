/* material_single_bxdf.hpp
 *
 * contains a definition for a material consisting of a single BXDF
 */

#ifndef RURT_MATERIAL_SINGLE_BXDF_H
#define RURT_MATERIAL_SINGLE_BXDF_H

#include "../material.hpp"
#include "../texture.hpp"

//-------------------------------------------//

namespace rurt
{

class MaterialSingleBXDF : public Material
{
public:
	MaterialSingleBXDF(const std::string& name, std::shared_ptr<BXDF> bxdf, const std::shared_ptr<Texture<vec3>>& color);

	vec3 bsdf_f(const IntersectionInfo& hitInfo, const vec3& wiWorld, const vec3& woWorld) const override;
	vec3 bsdf_sample_f(const IntersectionInfo& hitInfo, vec3& wiWorld, const vec3& woWorld, const vec3& u, float& pdf) const override;
	float bsdf_pdf(const IntersectionInfo& hitInfo, const vec3& wiWorld, const vec3& woWorld) const override;

private:
	std::shared_ptr<const Texture<vec3>> m_color;
	std::shared_ptr<const BXDF> m_bxdf;
};

}; //namespace rurt

#endif //#ifndef RURT_MATERIAL_SINGLE_BXDF_H