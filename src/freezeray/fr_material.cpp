#include "freezeray/fr_material.hpp"

#include "freezeray/fr_globals.hpp"
#include "freezeray/quickobj.h"
#include "freezeray/material/fr_material_mtl.hpp"
#include "freezeray/texture/fr_texture_constant.hpp"
#include "freezeray/texture/fr_texture_image.hpp"
#include "freezeray/texture/stb_image.h"
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

std::shared_ptr<const Texture<float>> Material::get_alpha_mask() const
{
	return nullptr;
}

std::vector<std::shared_ptr<const Material>> Material::from_mtl(const std::string& path, bool opacityIsMask)
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
		std::shared_ptr<Texture<float>> opacityTex;
		std::shared_ptr<Texture<float>> roughnessTex;
		std::shared_ptr<Texture<float>> etaTex;

		bool diffuseHasAlpha = false;

		//create textures:
		vec3 diffuseColor = vec3(material.diffuseColor.r, material.diffuseColor.g, material.diffuseColor.b);
		diffuseColor = srgb_to_linear(diffuseColor);
		if(material.diffuseMapPath)
		{
			//check if diffuse texture has alpha
			int width;
			int height;
			int numComp;
			stbi_info((prefix + material.diffuseMapPath).c_str(), &width, &height, &numComp);
			
			if(numComp >= 4)
				diffuseHasAlpha = true;

			//create texture
			diffuseColorTex = TextureImage<vec3, uint32_t, vec4>::from_file(
				prefix + material.diffuseMapPath, false, TextureRepeatMode::REPEAT, diffuseColor
			);
		}
		else
			diffuseColorTex = std::make_shared<TextureConstant<vec3>>(diffuseColor);

		vec3 specularColor = vec3(material.specularColor.r, material.specularColor.g, material.specularColor.b);
		specularColor = srgb_to_linear(specularColor);
		if(material.specularMapPath)
			specularColorTex = TextureImage<vec3, uint32_t>::from_file(
				prefix + material.specularMapPath, false, TextureRepeatMode::REPEAT, specularColor
			);
		else
			specularColorTex = std::make_shared<TextureConstant<vec3>>(specularColor);

		vec3 transmittanceColor = vec3(material.transmittanceColor.r, material.transmittanceColor.g, material.transmittanceColor.b);
		transmittanceColor = srgb_to_linear(transmittanceColor);
		if(material.transmittanceMapPath)
			transmittanceColorTex = TextureImage<vec3, uint32_t>::from_file(
				prefix + material.transmittanceMapPath, false, TextureRepeatMode::REPEAT, transmittanceColor
			);
		else
			transmittanceColorTex = std::make_shared<TextureConstant<vec3>>(transmittanceColor);

		if(material.opacityMapPath)
			opacityTex = TextureImage<float, uint8_t>::from_file(
				prefix + material.opacityMapPath, false, TextureRepeatMode::REPEAT, material.opacity
			);
		else if(diffuseHasAlpha)
		{
			std::shared_ptr<TextureImage<vec3, uint32_t, vec4>> diffuseTexImage = 
				std::dynamic_pointer_cast<TextureImage<vec3, uint32_t, vec4>>(diffuseColorTex);

			opacityTex = std::make_shared<TextureImage<float, uint32_t, vec3>>(
				diffuseTexImage->get_mip_pyramid(), diffuseTexImage->get_repeat_mode(), material.opacity
			);
		}
		else
			opacityTex = std::make_shared<TextureConstant<float>>(material.opacity);
		
		float eta = material.refractionIndex;
		float roughness = material.specularExp == 0.0f ? 0.0f : 1.0f / material.specularExp;
		roughness = std::max(std::min(roughness, 0.0f), 1.0f);

		//create material:
		std::shared_ptr<Material> uberMaterial = std::make_shared<MaterialMTL>(
			material.name,
			diffuseColorTex, specularColorTex, transmittanceColorTex,
			opacityTex, roughness, eta,
			opacityIsMask
		);

		result.push_back(uberMaterial);
	}

	//cleanup + return:
	//---------------
	qobj_free_mtl(numMaterials, materials);

	return result;
}

}; //namespace fr