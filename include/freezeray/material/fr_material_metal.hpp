/* fr_material_metal.hpp
 *
 * contains a definition for a metallic material
 */

#ifndef FR_MATERIAL_METAL_H
#define FR_MATERIAL_METAL_H

#include "../fr_material.hpp"
#include "../fr_texture.hpp"

//-------------------------------------------//

namespace fr
{

enum class MetalType : uint32_t
{
	ALUMINUM = 0,
	BRASS,
	COPPER,
	GOLD,
	IRON,
	LEAD,
	MERCURY,
	PLATINUM,
	SILVER,
	TITANIUM
};

class MaterialMetal : public Material
{
public:
	MaterialMetal(const std::string& name, const std::shared_ptr<Texture<vec3>>& eta, const std::shared_ptr<Texture<vec3>>& k, 
	              const std::shared_ptr<Texture<float>>& roughnessU, const std::shared_ptr<Texture<float>>& roughnessV);
	MaterialMetal(const std::string& name, const MetalType& type, 
	              const std::shared_ptr<Texture<float>>& roughnessU, const std::shared_ptr<Texture<float>>& roughnessV);

	vec3 bsdf_f(const IntersectionInfo& hitInfo, const vec3& wiWorld, const vec3& woWorld) const override;
	vec3 bsdf_sample_f(const IntersectionInfo& hitInfo, vec3& wiWorld, const vec3& woWorld, const vec3& u, float& pdf) const override;
	float bsdf_pdf(const IntersectionInfo& hitInfo, const vec3& wiWorld, const vec3& woWorld) const override;

private:
	std::shared_ptr<const Texture<vec3>> m_etaT;
	std::shared_ptr<const Texture<vec3>> m_absorption;
	std::shared_ptr<const Texture<float>> m_roughnessX;
	std::shared_ptr<const Texture<float>> m_roughnessY;
};

}; //namespace fr

#endif //#ifndef FR_MATERIAL_METAL_H