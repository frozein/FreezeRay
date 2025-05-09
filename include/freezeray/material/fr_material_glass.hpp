/* fr_material_glass.hpp
 *
 * contains a definition for a glass material with variable roughness
 * (weighted by fresnel terms)
 */

#ifndef FR_MATERIAL_GLASS_H
#define FR_MATERIAL_GLASS_H

#include "../fr_material.hpp"
#include "../fr_texture.hpp"
#include "../fresnel/fr_fresnel_dielectric.hpp"

//-------------------------------------------//

namespace fr
{

class MaterialGlass : public Material
{
public:
	MaterialGlass(const std::string& name, float eta, const std::shared_ptr<Texture<vec3>>& colorReflection, const std::shared_ptr<Texture<vec3>>& colorTransmission, 
	              const std::shared_ptr<Texture<float>>& roughnessX, const std::shared_ptr<Texture<float>>& roughnessY);

	std::shared_ptr<BSDF> get_bsdf(const IntersectionInfo& hitInfo) const;

private:
	float m_etaT;
	std::shared_ptr<const Texture<vec3>> m_colorReflection;
	std::shared_ptr<const Texture<vec3>> m_colorTransmission;
	std::shared_ptr<const Texture<float>> m_roughnessX;
	std::shared_ptr<const Texture<float>> m_roughnessY;
};

}; //namespace fr

#endif //#ifndef FR_MATERIAL_GLASS_H