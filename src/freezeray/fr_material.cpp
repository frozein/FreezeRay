#include "freezeray/fr_material.hpp"

#include "freezeray/fr_globals.hpp"
#include "freezeray/quickobj.h"

//-------------------------------------------//

namespace fr
{

Material::Material(const std::string& name) :
	m_name(name)
{

}

const std::string& Material::get_name() const 
{ 
	return m_name; 	
};

void Material::set_name(const std::string& name) 
{ 
	m_name = name; 
};

std::vector<std::shared_ptr<const Material>> Material::from_mtl(const std::string& path)
{
	//TODO
	return {};
}

}; //namespace fr