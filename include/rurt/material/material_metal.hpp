/* material_metal.hpp
 *
 * contains a definition for a metallic material
 */

#ifndef RURT_MATERIAL_METAL_H
#define RURT_MATERIAL_METAL_H

#include "../material.hpp"
#include "../bxdf/brdf_microfacet.hpp"
#include "../microfacet_distribution/microfacet_distribution_trowbridge_reitz.hpp"
#include "../fresnel/fresnel_conductor.hpp"

//-------------------------------------------//

namespace rurt
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
	MaterialMetal(const std::string& name, const vec3& eta, const vec3& k, float roughnessU, float roughnessV);
	MaterialMetal(const std::string& name, const MetalType& type, float roughnessU, float roughnessV);

	vec3 bsdf_f(const HitInfo& hitInfo, const vec3& wiWorld, const vec3& woWorld) const override;
	vec3 bsdf_sample_f(const HitInfo& hitInfo, vec3& wiWorld, const vec3& woWorld, const vec2& u, float& pdf) const override;
	float bsdf_pdf(const HitInfo& hitInfo, const vec3& wiWorld, const vec3& woWorld) const override;

private:
	FresnelConductor m_fresnel;
	MicrofacetDistributionTrowbridgeReitz m_distribution;
	BRDFMicrofacet m_brdf;
};

}; //namespace rurt

#endif //#ifndef RURT_MATERIAL_METAL_H