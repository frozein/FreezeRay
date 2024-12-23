/* material_glass.hpp
 *
 * contains a definition for a glass material with variable roughness
 * (weighted by fresnel terms)
 */

#ifndef RURT_MATERIAL_GLASS_H
#define RURT_MATERIAL_GLASS_H

#include "../material.hpp"
#include "../bxdf/brdf_microfacet.hpp"
#include "../bxdf/btdf_microfacet.hpp"
#include "../fresnel/fresnel_dielectric.hpp"
#include "../microfacet_distribution/microfacet_distribution_trowbridge_reitz.hpp"

//-------------------------------------------//

namespace rurt
{

class MaterialGlass : public Material
{
public:
	MaterialGlass(const std::string& name, const vec3& colorReflection, const vec3& colorTransmission, float roughnessU, float roughnessV);

	vec3 bsdf_f(const IntersectionInfo& hitInfo, const vec3& wiWorld, const vec3& woWorld) const override;
	vec3 bsdf_sample_f(const IntersectionInfo& hitInfo, vec3& wiWorld, const vec3& woWorld, const vec2& u, float& pdf) const override;
	float bsdf_pdf(const IntersectionInfo& hitInfo, const vec3& wiWorld, const vec3& woWorld) const override;

private:
	FresnelDielectric m_fresnel;
	MicrofacetDistributionTrowbridgeReitz m_distribution;
	BRDFMicrofacet m_brdf;
	BTDFMicrofacet m_btdf;

	vec3 m_colorReflection;
	vec3 m_colorTransmission;
};

}; //namespace rurt

#endif //#ifndef RURT_MATERIAL_GLASS_H