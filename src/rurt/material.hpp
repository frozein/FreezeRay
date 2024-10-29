/* material.hpp
 *
 * contains the definition of the material class, which represents
 * aspects of a particular type of surface, as well as info on how rays
 * should interact with it
 */

#ifndef RURT_MATERIAL_H
#define RURT_MATERIAL_H

#include <string>
#include <memory>

//-------------------------------------------//

namespace rurt
{

class Material
{
public:
	Material(std::string name);

	std::string get_name();
	void set_name(std::string name);

	static std::shared_ptr<Material> default_diffuse();

private:
	static std::shared_ptr<Material> m_defaultDiffuse;

	std::string m_name;
};

}; //namespace rurt

#endif //#ifndef RURT_MATERIAL_H