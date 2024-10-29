#include "object.hpp"

//-------------------------------------------//

namespace rurt
{

Object::Object(std::vector<std::shared_ptr<Mesh>> meshes, std::vector<std::shared_ptr<Material>> materials)
{
	//assign each mesh its material, add to vector:
	//---------------
	for(uint32_t i = 0; i < meshes.size(); i++)
	{
		std::shared_ptr<Material> mat = nullptr;
		for(uint32_t j = 0; j < materials.size(); j++)
		{
			if(meshes[i]->get_material() == materials[i]->get_name())
				mat = materials[i];
		}

		//set to default material if not found
		if(mat == nullptr)
			mat = Material::default_diffuse();

		m_meshes.push_back({meshes[i], mat});
	}
}

bool Object::intersect(const Ray& ray, float& minT, vec2& uv, vec3& normal)
{
	//loop over every mesh, check for intersection:
	//---------------
	bool hit = false;
	minT = INFINITY;

	for(uint32_t i = 0; i < m_meshes.size(); i++)
	{
		float t;
		vec2 newUV;
		vec3 newNormal;
		if(m_meshes[i].first->intersect(ray, t, newUV, newNormal) && t < minT)
		{
			hit |= true;
			minT = t;
			uv = newUV;
			normal = newNormal;
		}
	}

	return hit;
}

}; //namespace rurt