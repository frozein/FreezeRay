#include "freezeray/material/fr_material_uber.hpp"
#include "freezeray/fr_globals.hpp"
#include "freezeray/bxdf/fr_brdf_microfacet.hpp"
#include "freezeray/bxdf/fr_btdf_microfacet.hpp"
#include "freezeray/bxdf/fr_brdf_lambertian.hpp"
#include "freezeray/bxdf/fr_brdf_specular.hpp"
#include "freezeray/bxdf/fr_btdf_specular.hpp"
#include "freezeray/fresnel/fr_fresnel_dielectric.hpp"
#include "freezeray/fresnel/fr_fresnel_constant.hpp"
#include "freezeray/microfacet_distribution/fr_microfacet_distribution_trowbridge_reitz.hpp"
#include <math.h>

//-------------------------------------------//

namespace fr
{

#define ETA_I 1.0f //refractive index of air

//-------------------------------------------//

MaterialUber::MaterialUber(const std::string& name, std::shared_ptr<const Texture<vec3>> colorDiffuse, std::shared_ptr<const Texture<vec3>> colorSpecular,
                           std::shared_ptr<const Texture<vec3>> colorTransmittance, std::shared_ptr<const Texture<float>> roughnessX,
                           std::shared_ptr<const Texture<float>> roughnessY, std::shared_ptr<const Texture<vec3>> opacity,
                           std::shared_ptr<const Texture<float>> eta) :
	Material(name), m_colorDiffuse(colorDiffuse), m_colorSpecular(colorSpecular),
	m_colorTransmittance(colorTransmittance), m_roughnessX(roughnessX), m_roughnessY(roughnessY),
	m_opacity(opacity), m_eta(eta)
{

}

std::shared_ptr<BSDF> MaterialUber::get_bsdf(const IntersectionInfo& hitInfo) const
{
	//sample textures:
	//---------------
	vec3 opacity = m_opacity->evaluate(hitInfo);
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

	//TODO: is this stupid? come up with a better soln
	std::shared_ptr<Fresnel> fresnel; //ignore eta for fully opaque materials
	if(opacity == 1.0f) 
		fresnel = std::make_shared<FresnelConstant>(1.0f);
	else
		fresnel = std::make_shared<FresnelDielectric>(ETA_I, eta);

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

	//TODO: microfacet or specular transmission?
	if(colorTransmittance != 0.0f)
		bsdf->add_bxdf(
			std::make_shared<BTDFMicrofacet>(ETA_I, eta, distribution, fresnel),
			colorTransmittance
		);

	return bsdf;
}


}; // namespace fr