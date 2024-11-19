/* material_single_bxdf.hpp
 *
 * contains a definition for a material consisting of a single BXDF
 */

#ifndef RURT_MATERIAL_SINGLE_BXDF_H
#define RURT_MATERIAL_SINGLE_BXDF_H

#include "material.hpp"

//-------------------------------------------//

namespace rurt
{

class MaterialSingleBXDF : public Material
{
public:
	MaterialSingleBXDF(const std::string& name, std::shared_ptr<BXDF> bxdf) : Material(name), m_bxdf(bxdf) { }

	std::shared_ptr<BXDF> get_bxdf() const override { return m_bxdf; }
	vec3 get_emission() const override { return vec3(0.0f); }

private:
	std::shared_ptr<BXDF> m_bxdf;
};

}; //namespace rurt

#endif //#ifndef RURT_MATERIAL_SINGLE_BXDF_H