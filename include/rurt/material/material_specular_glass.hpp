/* material_specular_glass.hpp
 *
 * contains a definition for a perfectly specular glass material
 * (weighted by fresnel terms)
 */

#ifndef RURT_MATERIAL_SPECULAR_GLASS_H
#define RURT_MATERIAL_SPECULAR_GLASS_H

#include "../material.hpp"
#include "../texture.hpp"
#include "../bxdf/brdf_specular.hpp"
#include "../bxdf/btdf_specular.hpp"
#include "../fresnel/fresnel_dielectric.hpp"

//-------------------------------------------//

namespace rurt
{

class MaterialSpecularGlass : public Material
{
public:
	MaterialSpecularGlass(const std::string& name, float eta, const std::shared_ptr<Texture<vec3>>& colorReflection, const std::shared_ptr<Texture<vec3>>& colorTransmission);

	vec3 bsdf_f(const IntersectionInfo& hitInfo, const vec3& wiWorld, const vec3& woWorld) const override;
	vec3 bsdf_sample_f(const IntersectionInfo& hitInfo, vec3& wiWorld, const vec3& woWorld, const vec3& u, float& pdf) const override;
	float bsdf_pdf(const IntersectionInfo& hitInfo, const vec3& wiWorld, const vec3& woWorld) const override;

private:
	float m_etaT;
	std::shared_ptr<const Texture<vec3>> m_colorReflection;
	std::shared_ptr<const Texture<vec3>> m_colorTransmission;

	FresnelDielectric m_fresnel;
	BRDFSpecular m_brdf;
	BTDFSpecular m_btdf;
};

}; //namespace rurt

#endif //#ifndef RURT_MATERIAL_SPECULAR_GLASS_H