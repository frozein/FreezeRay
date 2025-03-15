/* fr_material_specular_glass.hpp
 *
 * contains a definition for a perfectly specular glass material
 * (weighted by fresnel terms)
 */

#ifndef FR_MATERIAL_SPECULAR_GLASS_H
#define FR_MATERIAL_SPECULAR_GLASS_H

#include "../fr_material.hpp"
#include "../fr_texture.hpp"
#include "../bxdf/fr_brdf_specular.hpp"
#include "../bxdf/fr_btdf_specular.hpp"
#include "../fresnel/fr_fresnel_dielectric.hpp"

//-------------------------------------------//

namespace fr
{

class MaterialSpecularGlass : public Material
{
public:
	MaterialSpecularGlass(const std::string& name, float eta, const std::shared_ptr<Texture<vec3>>& colorReflection, const std::shared_ptr<Texture<vec3>>& colorTransmission);

	std::shared_ptr<BSDF> get_bsdf(const IntersectionInfo& hitInfo) const;

private:
	float m_etaT;
	std::shared_ptr<const Texture<vec3>> m_colorReflection;
	std::shared_ptr<const Texture<vec3>> m_colorTransmission;
};

}; //namespace fr

#endif //#ifndef FR_MATERIAL_SPECULAR_GLASS_H