/* material_glass.hpp
 *
 * contains a definition for a perfectly specular glass material
 * (weighted by fresnel terms)
 */

#ifndef RURT_MATERIAL_SPECULAR_GLASS_H
#define RURT_MATERIAL_SPECULAR_GLASS_H

#include "../material.hpp"
#include "../bxdf/brdf_specular.hpp"
#include "../bxdf/btdf_specular.hpp"
#include "../fresnel/fresnel_dielectric.hpp"

//-------------------------------------------//

namespace rurt
{

class MaterialSpecularGlass : public Material
{
public:
	MaterialSpecularGlass(const std::string& name, const vec3& color);

	vec3 bsdf_f(const HitInfo& hitInfo, const vec3& wiWorld, const vec3& woWorld) const override;
	vec3 bsdf_sample_f(const HitInfo& hitInfo, vec3& wiWorld, const vec3& woWorld, const vec2& u, float& pdf) const override;
	float bsdf_pdf(const HitInfo& hitInfo, const vec3& wiWorld, const vec3& woWorld) const override;

private:
	FresnelDielectric m_fresnel;
	BRDFSpecular m_brdf;
	BTDFSpecular m_btdf;

	vec3 m_color;
};

}; //namespace rurt

#endif //#ifndef RURT_MATERIAL_SPECULAR_GLASS_H