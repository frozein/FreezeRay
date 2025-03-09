/* fr_material_plastic.hpp
 *
 * contains a definition for a plastic material
 */

#ifndef FR_MATERIAL_PASTIC_H
#define FR_MATERIAL_PASTIC_H

#include "../fr_material.hpp"
#include "../fr_texture.hpp"
#include "../bxdf/fr_brdf_lambertian.hpp"
#include "../fresnel/fr_fresnel_dielectric.hpp"

//-------------------------------------------//

namespace fr
{

class MaterialPlastic : public Material
{
public:
	MaterialPlastic(const std::string& name, const std::shared_ptr<Texture<vec3>>& colorDiffuse, const std::shared_ptr<Texture<vec3>>& colorSpecular, 
	                const std::shared_ptr<Texture<float>>& roughness);

	vec3 bsdf_f(const IntersectionInfo& hitInfo, const vec3& wiWorld, const vec3& woWorld) const override;
	vec3 bsdf_sample_f(const IntersectionInfo& hitInfo, vec3& wiWorld, const vec3& woWorld, const vec3& u, float& pdf) const override;
	float bsdf_pdf(const IntersectionInfo& hitInfo, const vec3& wiWorld, const vec3& woWorld) const override;

private:
	std::shared_ptr<const Texture<vec3>> m_colorDiffuse;
	std::shared_ptr<const Texture<vec3>> m_colorSpecular;
	std::shared_ptr<const Texture<float>> m_roughness;

	FresnelDielectric m_fresnel;
	BRDFLambertian m_brdfDiffuse;
};

}; //namespace fr

#endif //#ifndef FR_MATERIAL_PLASTIC_H