#include "material.hpp"

//-------------------------------------------//

namespace rurt
{

std::shared_ptr<Material> Material::m_defaultDiffuse = std::make_shared<Material>("diffuse");

//-------------------------------------------//

Material::Material(std::string name)
{
	m_name = name;
}

std::string Material::get_name()
{
	return m_name;
}

void Material::set_name(std::string name)
{
	m_name = name;
}

std::shared_ptr<Material> Material::default_diffuse()
{
	return m_defaultDiffuse;
}

}; //namespace rurt