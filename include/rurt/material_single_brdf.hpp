/* material_single_brdf.hpp
 *
 * contains a definition for a material consisting of a single BRDF
 */

#ifndef RURT_MATERIAL_SINGLE_BRDF_H
#define RURT_MATERIAL_SINGLE_BRDF_H

#include "material.hpp"

//-------------------------------------------//

namespace rurt
{

class MaterialSingleBRDF : public Material
{
public:
	MaterialSingleBRDF(const std::string& name, std::shared_ptr<BRDF> brdf) : Material(name), m_brdf(brdf) { }

	std::shared_ptr<BRDF> get_brdf() const override { return m_brdf; }
	vec3 get_emission() const override { return vec3(0.0f); }

private:
	std::shared_ptr<BRDF> m_brdf;
};

}; //namespace rurt

#endif //#ifndef RURT_MATERIAL_SINGLE_BRDF_H