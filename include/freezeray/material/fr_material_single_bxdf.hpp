/* fr_material_single_bxdf.hpp
 *
 * contains a definition for a material consisting of a single BXDF
 */

#ifndef FR_MATERIAL_SINGLE_BXDF_H
#define FR_MATERIAL_SINGLE_BXDF_H

#include "../fr_material.hpp"
#include "../fr_texture.hpp"

//-------------------------------------------//

namespace fr
{

class MaterialSingleBXDF : public Material
{
public:
	MaterialSingleBXDF(const std::string& name, std::shared_ptr<BXDF> bxdf, const std::shared_ptr<Texture<vec3>>& color);

	std::shared_ptr<BSDF> get_bsdf(const IntersectionInfo& hitInfo) const;

private:
	std::shared_ptr<const Texture<vec3>> m_color;
	std::shared_ptr<const BXDF> m_bxdf;
};

}; //namespace fr

#endif //#ifndef FR_MATERIAL_SINGLE_BXDF_H