/* fr_material_uber.hpp
 *
 * contains a definition for a material from an OBJ MTL file
 */

#ifndef FR_MATERIAL_UBER_H
#define FR_MATERIAL_UBER_H

#include "../fr_material.hpp"
#include "../fr_texture.hpp"

//-------------------------------------------//

namespace fr
{

class MaterialMTL : public Material
{
public:
	MaterialMTL(const std::string& name, std::shared_ptr<const Texture<vec3>> colorDiffuse, std::shared_ptr<const Texture<vec3>> colorSpecular,
	            std::shared_ptr<const Texture<vec3>> colorTransmittance, std::shared_ptr<const Texture<float>> opacity,
	            float roughness, float eta, bool opacityIsMask = true);

	std::shared_ptr<BSDF> get_bsdf(const IntersectionInfo& hitInfo) const override;
	std::shared_ptr<const Texture<float>> get_alpha_mask() const override;
	
private:
	std::shared_ptr<const Texture<vec3>> m_colorDiffuse;
	std::shared_ptr<const Texture<vec3>> m_colorSpecular;
	std::shared_ptr<const Texture<vec3>> m_colorTransmittance;
	std::shared_ptr<const Texture<float>> m_opacity;
	float m_eta;
	float m_roughness;
	bool m_opacityIsMask;
};

}; //namespace fr

#endif