/* material_plastic.hpp
 *
 * contains a definition for a plastic material
 */

#ifndef RURT_MATERIAL_PASTIC_H
#define RURT_MATERIAL_PASTIC_H

#include "../material.hpp"
#include "../bxdf/brdf_lambertian_diffuse.hpp"
#include "../bxdf/brdf_microfacet.hpp"
#include "../microfacet_distribution/microfacet_distribution_trowbridge_reitz.hpp"
#include "../fresnel/fresnel_dielectric.hpp"

//-------------------------------------------//

namespace rurt
{

class MaterialPlastic : public Material
{
public:
	MaterialPlastic(const std::string& name, const vec3& colorDiffuse, const vec3& colorSpecular, float roughness);

	vec3 bsdf_f(const HitInfo& hitInfo, const vec3& wiWorld, const vec3& woWorld) const override;
	vec3 bsdf_sample_f(const HitInfo& hitInfo, vec3& wiWorld, const vec3& woWorld, const vec2& u, float& pdf) const override;
	float bsdf_pdf(const HitInfo& hitInfo, const vec3& wiWorld, const vec3& woWorld) const override;

private:
	FresnelDielectric m_fresnel;
	MicrofacetDistributionTrowbridgeReitz m_distribution;
	BRDFLambertianDiffuse m_brdfDiffuse;
	BRDFMicrofacet m_brdfSpecular;

	vec3 m_colorDiffuse;
	vec3 m_colorSpecular;
};

}; //namespace rurt

#endif //#ifndef RURT_MATERIAL_PLASTIC_H