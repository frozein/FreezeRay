#include "freezeray/fr_object.hpp"

//-------------------------------------------//

namespace fr
{

Object::Object(const std::vector<std::shared_ptr<const Mesh>>& meshes, const std::vector<std::shared_ptr<const Material>>& materials)
{
	//assign each mesh its material, add to vector:
	//---------------
	for(uint32_t i = 0; i < meshes.size(); i++)
	{
		std::shared_ptr<const Material> mat = nullptr;
		for(uint32_t j = 0; j < materials.size(); j++)
		{
			if(meshes[i]->get_material() == materials[j]->get_name())
				mat = materials[j];
		}

		//set to default material if not found
		if(mat == nullptr)
		{
			std::cout << "ERROR: no material found" << std::endl;
			//TODO!!!!
		}

		m_components.push_back({meshes[i], mat});
	}
}

Object::Object(const std::shared_ptr<const Mesh>& mesh, const std::shared_ptr<const Material>& material) :
	Object(std::vector<std::shared_ptr<const Mesh>>({mesh}), std::vector<std::shared_ptr<const Material>>({material}))
{

}

Object::Object(const std::vector<ObjectComponent>& components) :
	m_components(components)
{

}

bool Object::intersect(const Ray& ray, float& minT, vec2& uv, vec3& normal, IntersectionInfo::Derivatives& derivs, std::shared_ptr<const Material>& material) const
{
	//loop over every mesh, check for intersection:
	//---------------
	bool hit = false;
	minT = INFINITY;

	for(uint32_t i = 0; i < m_components.size(); i++)
	{
		std::shared_ptr<const Material> mat = m_components[i].material;

		float t;
		vec2 newUV;
		vec3 newNormal;
		IntersectionInfo::Derivatives newDerivs;
		if(m_components[i].mesh->intersect(ray, mat->get_alpha_mask(), t, newUV, newNormal, newDerivs) && t < minT)
		{
			hit |= true;
			minT = t;
			uv = newUV;
			normal = newNormal;
			material = mat;
			derivs = newDerivs;
		}
	}

	return hit;
}

std::shared_ptr<const Object> Object::from_obj(const std::string& objPath, const std::string& mtlPath, bool opacityIsMask)
{
	std::vector<std::shared_ptr<const Mesh>> meshes = Mesh::from_obj(objPath);
	std::vector<std::shared_ptr<const Material>> materials = Material::from_mtl(mtlPath, opacityIsMask);

	return std::make_shared<Object>(meshes, materials);
}

}; //namespace fr