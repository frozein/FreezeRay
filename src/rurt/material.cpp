#include "material.hpp"

//-------------------------------------------//

namespace rurt
{

std::shared_ptr<Material> Material::m_defaultDiffuse = std::make_shared<Material>("diffuse");

//-------------------------------------------//

Material::Material(const std::string& name)
{
	m_name = name;
}

const std::string& Material::get_name() const
{
	return m_name;
}

void Material::set_name(const std::string& name)
{
	m_name = name;
}

std::shared_ptr<Material> Material::default_diffuse()
{
	return m_defaultDiffuse;
}

}; //namespace rurt