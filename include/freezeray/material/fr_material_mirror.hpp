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

	std::shared_ptr<BSDF> get_bsdf(const IntersectionInfo& hitInfo) const;

private:
	std::shared_ptr<const Texture<vec3>> m_color;
};

};

#endif //#ifndef FR_MATERIAL_MIRROR