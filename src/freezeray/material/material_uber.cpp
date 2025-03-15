#include "freezeray/material/fr_material_uber.hpp"
#include "freezeray/fr_globals.hpp"
#include "freezeray/bxdf/fr_brdf_microfacet.hpp"
#include "freezeray/bxdf/fr_btdf_microfacet.hpp"
#include "freezeray/bxdf/fr_brdf_lambertian.hpp"
#include "freezeray/bxdf/fr_brdf_specular.hpp"
#include "freezeray/bxdf/fr_btdf_specular.hpp"
#include "freezeray/fresnel/fr_fresnel_dielectric.hpp"
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
	//TODO

	return nullptr;
}


}; // namespace fr