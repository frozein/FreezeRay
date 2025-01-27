/* fr_material_mirror.hpp
 *
 * contains a definition for a perfect mirror material
 */

#ifndef FR_MATERIAL_MIRROR
#define FR_MATERIAL_MIRROR

#include "../fr_material.hpp"
#include "../fr_texture.hpp"
#include "../bxdf/fr_brdf_specular.hpp"
#include "../fresnel/fr_fresnel_constant.hpp"

//-------------------------------------------//

namespace fr
{

class MaterialMirror : public Material
{
public:
	MaterialMirror(const std::string& name, const std::shared_ptr<Texture<vec3>>& color);

	vec3 bsdf_f(const IntersectionInfo& hitInfo, const vec3& wiWorld, const vec3& woWorld) const override;
	vec3 bsdf_sample_f(const IntersectionInfo& hitInfo, vec3& wiWorld, const vec3& woWorld, const vec3& u, float& pdf) const override;
	float bsdf_pdf(const IntersectionInfo& hitInfo, const vec3& wiWorld, const vec3& woWorld) const override;

private:
	std::shared_ptr<const Texture<vec3>> m_color;

	FresnelConstant m_fresnel;
	BRDFSpecular m_brdf;
};

};

#endif //#ifndef FR_MATERIAL_MIRROR