/* material_mirror.hpp
 *
 * contains a definition for a perfect mirror material
 */

#ifndef RURT_MATERIAL_MIRROR
#define RURT_MATERIAL_MIRROR

#include "../material.hpp"
#include "../bxdf/brdf_specular.hpp"
#include "../fresnel/fresnel_constant.hpp"

//-------------------------------------------//

namespace rurt
{

class MaterialMirror : public Material
{
public:
	MaterialMirror(const std::string& name, const vec3& color);

	vec3 bsdf_f(const HitInfo& hitInfo, const vec3& wiWorld, const vec3& woWorld) const override;
	vec3 bsdf_sample_f(const HitInfo& hitInfo, vec3& wiWorld, const vec3& woWorld, const vec2& u, float& pdf) const override;
	float bsdf_pdf(const HitInfo& hitInfo, const vec3& wiWorld, const vec3& woWorld) const override;

private:
	FresnelConstant m_fresnel;
	BRDFSpecular m_brdf;

	vec3 m_color;
};

};

#endif //#ifndef RURT_MATERIAL_MIRROR