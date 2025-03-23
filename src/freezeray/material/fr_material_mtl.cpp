#include "freezeray/material/fr_material_mtl.hpp"

#include "freezeray/fr_globals.hpp"
#include "freezeray/bxdf/fr_brdf_microfacet.hpp"
#include "freezeray/bxdf/fr_btdf_microfacet.hpp"
#include "freezeray/bxdf/fr_brdf_lambertian.hpp"
#include "freezeray/bxdf/fr_brdf_specular.hpp"
#include "freezeray/bxdf/fr_btdf_specular.hpp"
#include "freezeray/bxdf/fr_btdf_passthrough.hpp"
#include "freezeray/fresnel/fr_fresnel_dielectric.hpp"
#include "freezeray/fresnel/fr_fresnel_constant.hpp"
#include "freezeray/microfacet_distribution/fr_microfacet_distribution_trowbridge_reitz.hpp"
#include <math.h>

//-------------------------------------------//

namespace fr
{

#define ETA_I 1.0f //refractive index of air

//-------------------------------------------//

MaterialMTL::MaterialMTL(const std::string& name, std::shared_ptr<const Texture<vec3>> colorDiffuse, std::shared_ptr<const Texture<vec3>> colorSpecular,
                           std::shared_ptr<const Texture<vec3>> colorTransmittance, std::shared_ptr<const Texture<float>> roughnessX,
                           std::shared_ptr<const Texture<float>> roughnessY, std::shared_ptr<const Texture<float>> opacity,
                           std::shared_ptr<const Texture<float>> eta) :
	Material(name), m_colorDiffuse(colorDiffuse), m_colorSpecular(colorSpecular),
	m_colorTransmittance(colorTransmittance), m_roughnessX(roughnessX), m_roughnessY(roughnessY),
	m_opacity(opacity), m_eta(eta)
{

}

std::shared_ptr<BSDF> MaterialMTL::get_bsdf(const IntersectionInfo& hitInfo) const
{
	//sample textures:
	//---------------
	float opacity = m_opacity->evaluate(hitInfo);
	float eta = m_eta->evaluate(hitInfo);

	vec3 colorDiffuse       =         opacity  * m_colorDiffuse->evaluate(hitInfo);
	vec3 colorSpecular      =         opacity  * m_colorSpecular->evaluate(hitInfo);
	vec3 colorTransmittance = (1.0f - opacity) * m_colorTransmittance->evaluate(hitInfo);
	
	float roughnessX = m_roughnessX->evaluate(hitInfo);
	float roughnessY = m_roughnessY->evaluate(hitInfo);

	//create distribution + fresnel:
	//---------------
	std::shared_ptr<MicrofacetDistribution> distribution = 
		std::make_shared<MicrofacetDistributionTrowbridgeReitz>(roughnessX, roughnessY);

	std::shared_ptr<Fresnel> fresnel = std::make_shared<FresnelDielectric>(ETA_I, eta);

	//create bsdf:
	//---------------
	std::shared_ptr<BSDF> bsdf = std::make_shared<BSDF>(hitInfo.worldNormal);

	if(colorDiffuse != 0.0f)
		bsdf->add_bxdf(
			std::make_shared<BRDFLambertian>(),
			colorDiffuse
		);

	if(colorSpecular != 0.0f)
		bsdf->add_bxdf(
			std::make_shared<BRDFMicrofacet>(distribution, fresnel), 
			colorSpecular
		);

	//TODO: do we always want to ignore refraction?
	if(colorTransmittance != 0.0f)
		bsdf->add_bxdf(
			std::make_shared<BTDFPassthrough>(),
			colorTransmittance
		);

	return bsdf;
}


}; // namespace fr