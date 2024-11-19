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
#include "bxdf.hpp"

//-------------------------------------------//

namespace rurt
{

class Material
{
public:
	Material(const std::string& name) : m_name(name) {};

	const std::string& get_name() const { return m_name; };
	void set_name(const std::string& name) { m_name = name; };

	virtual std::shared_ptr<BXDF> get_bxdf() const = 0;
	virtual vec3 get_emission() const = 0;

private:
	std::string m_name;
};

}; //namespace rurt

#endif //#ifndef RURT_MATERIAL_H