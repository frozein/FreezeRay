/* fr_material.hpp
 *
 * contains the definition of the material class, which represents
 * aspects of a particular type of surface, as well as info on how rays
 * should interact with it
 */

#ifndef FR_MATERIAL_H
#define FR_MATERIAL_H

#include <string>
#include <memory>
#include <vector>

#include "fr_bsdf.hpp"
#include "fr_raycast_info.hpp"

//-------------------------------------------//

namespace fr
{

class Material
{
public:
	Material(const std::string& name);

	const std::string& get_name() const;
	void set_name(const std::string& name);

	virtual std::shared_ptr<BSDF> get_bsdf(const IntersectionInfo& hitInfo) const = 0;

	static std::vector<std::shared_ptr<const Material>> from_mtl(const std::string& path);

private:
	std::string m_name;
};

}; //namespace fr

#endif //#ifndef FR_MATERIAL_H