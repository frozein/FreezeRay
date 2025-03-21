#include "freezeray/fr_material.hpp"

#include "freezeray/fr_globals.hpp"
#include "freezeray/quickobj.h"
#include "freezeray/material/fr_material_uber.hpp"
#include "freezeray/texture/fr_texture_constant.hpp"
#include "freezeray/texture/fr_texture_image.hpp"
#include <filesystem>

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
	std::vector<std::shared_ptr<const Material>> result = {};

	//get path to parent dir:
	//---------------
	std::filesystem::path fsPath(path);
	std::string prefix = fsPath.parent_path().string() + "/";
	
	//load materials with qobj:
	//---------------
	uint32_t numMaterials;
	QOBJmaterial* materials;

	QOBJerror err = qobj_load_mtl(path.c_str(), &numMaterials, &materials);
	if(err != QOBJ_SUCCESS)
		throw std::runtime_error("QOBJ failed to load .mtl file \"" + path + "\" with error " + std::to_string(err));

	//convert to material objects:
	//---------------
	for(uint32_t i = 0; i < numMaterials; i++)
	{
		QOBJmaterial material = materials[i];
		
		//define textures:
		std::shared_ptr<Texture<vec3>> diffuseColorTex;
		std::shared_ptr<Texture<vec3>> specularColorTex;
		std::shared_ptr<Texture<vec3>> transmittanceColorTex;
		std::shared_ptr<Texture<vec3>> opacityTex;
		std::shared_ptr<Texture<float>> roughnessTex;
		std::shared_ptr<Texture<float>> etaTex;

		//create textures:
		vec3 diffuseColor = vec3(material.diffuseColor.r, material.diffuseColor.g, material.diffuseColor.b);
		if(materials[i].diffuseMapPath)
			diffuseColorTex = TextureImage<vec3, uint32_t>::from_file(
				prefix + material.diffuseMapPath, false, TextureRepeatMode::REPEAT, diffuseColor
			);
		else
			diffuseColorTex = std::make_shared<TextureConstant<vec3>>(diffuseColor);

		vec3 specularColor = vec3(material.specularColor.r, material.specularColor.g, material.specularColor.b);
		if(materials[i].specularMapPath)
			specularColorTex = TextureImage<vec3, uint32_t>::from_file(
				prefix + material.specularMapPath, false, TextureRepeatMode::REPEAT, specularColor
			);
		else
			specularColorTex = std::make_shared<TextureConstant<vec3>>(specularColor);

		vec3 transmittanceColor = vec3(material.transmittanceColor.r, material.transmittanceColor.g, material.transmittanceColor.b);
		if(materials[i].transmittanceMapPath)
			transmittanceColorTex = TextureImage<vec3, uint32_t>::from_file(
				prefix + material.transmittanceMapPath, false, TextureRepeatMode::REPEAT, transmittanceColor
			);
		else
			transmittanceColorTex = std::make_shared<TextureConstant<vec3>>(transmittanceColor);

		opacityTex = std::make_shared<TextureConstant<vec3>>(material.opacity);
		
		float roughnessVal = material.specularExp == 0.0f ? 0.0f : 1.0f / material.specularExp;
		roughnessVal = std::min(std::max(roughnessVal, 0.0f), 1.0f);
		roughnessTex = std::make_shared<TextureConstant<float>>(roughnessVal);

		etaTex = std::make_shared<TextureConstant<float>>(material.refractionIndex);

		//create material:
		std::shared_ptr<Material> uberMaterial = std::make_shared<MaterialUber>(
			material.name,
			diffuseColorTex, specularColorTex, transmittanceColorTex,
			roughnessTex, roughnessTex,
			opacityTex, etaTex
		);

		result.push_back(uberMaterial);
	}

	//cleanup + return:
	//---------------
	qobj_free_mtl(numMaterials, materials);

	return result;
}

}; //namespace fr