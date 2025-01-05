/* material_glass.hpp
 *
 * contains a definition for a glass material with variable roughness
 * (weighted by fresnel terms)
 */

#ifndef RURT_MATERIAL_GLASS_H
#define RURT_MATERIAL_GLASS_H

#include "../material.hpp"
#include "../texture.hpp"
#include "../fresnel/fresnel_dielectric.hpp"

//-------------------------------------------//

namespace rurt
{

class MaterialGlass : public Material
{
public:
	MaterialGlass(const std::string& name, float eta, const std::shared_ptr<Texture<vec3>>& colorReflection, const std::shared_ptr<Texture<vec3>>& colorTransmission, 
	const std::shared_ptr<Texture<float>>& roughnessX, const std::shared_ptr<Texture<float>>& roughnessY);

	vec3 bsdf_f(const IntersectionInfo& hitInfo, const vec3& wiWorld, const vec3& woWorld) const override;
	vec3 bsdf_sample_f(const IntersectionInfo& hitInfo, vec3& wiWorld, const vec3& woWorld, const vec2& u, float& pdf) const override;
	float bsdf_pdf(const IntersectionInfo& hitInfo, const vec3& wiWorld, const vec3& woWorld) const override;

private:
	float m_etaT;
	std::shared_ptr<const Texture<vec3>> m_colorReflection;
	std::shared_ptr<const Texture<vec3>> m_colorTransmission;
	std::shared_ptr<const Texture<float>> m_roughnessX;
	std::shared_ptr<const Texture<float>> m_roughnessY;

	FresnelDielectric m_fresnel;
};

}; //namespace rurt

#endif //#ifndef RURT_MATERIAL_GLASS_H