/* material_plastic.hpp
 *
 * contains a definition for a plastic material
 */

#ifndef RURT_MATERIAL_PASTIC_H
#define RURT_MATERIAL_PASTIC_H

#include "../material.hpp"
#include "../texture.hpp"
#include "../bxdf/brdf_lambertian_diffuse.hpp"
#include "rurt/fresnel/fresnel_dielectric.hpp"

//-------------------------------------------//

namespace rurt
{

class MaterialPlastic : public Material
{
public:
	MaterialPlastic(const std::string& name, const std::shared_ptr<Texture<vec3>>& colorDiffuse, const std::shared_ptr<Texture<vec3>>& colorSpecular, 
	                const std::shared_ptr<Texture<float>>& roughness);

	vec3 bsdf_f(const IntersectionInfo& hitInfo, const vec3& wiWorld, const vec3& woWorld) const override;
	vec3 bsdf_sample_f(const IntersectionInfo& hitInfo, vec3& wiWorld, const vec3& woWorld, const vec2& u, float& pdf) const override;
	float bsdf_pdf(const IntersectionInfo& hitInfo, const vec3& wiWorld, const vec3& woWorld) const override;

private:
	std::shared_ptr<const Texture<vec3>> m_colorDiffuse;
	std::shared_ptr<const Texture<vec3>> m_colorSpecular;
	std::shared_ptr<const Texture<float>> m_roughness;

	FresnelDielectric m_fresnel;
	BRDFLambertianDiffuse m_brdfDiffuse;
};

}; //namespace rurt

#endif //#ifndef RURT_MATERIAL_PLASTIC_H