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

	std::shared_ptr<BSDF> get_bsdf(const IntersectionInfo& hitInfo) const;

private:
	std::shared_ptr<const Texture<vec3>> m_colorDiffuse;
	std::shared_ptr<const Texture<vec3>> m_colorSpecular;
	std::shared_ptr<const Texture<float>> m_roughness;
};

}; //namespace fr

#endif //#ifndef FR_MATERIAL_PLASTIC_H