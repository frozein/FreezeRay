/* object.hpp
 *
 * contains the definition of the object class, which represents
 * a single object (collection of meshes and materials)
 */

#ifndef RURT_OBJECT_H
#define RURT_OBJECT_H

#include "mesh.hpp"
#include "material.hpp"

//-------------------------------------------//

namespace rurt
{

class Object
{
public:
	Object(std::vector<std::shared_ptr<Mesh>> meshes, std::vector<std::shared_ptr<Material>> materials);

	bool intersect(const Ray& ray, float& t, vec2& uv, vec3& normal);

private:
	std::vector<std::pair<std::shared_ptr<Mesh>, std::shared_ptr<Material>>> m_meshes;
};

}; //namespace rurt

#endif //#ifndef RURT_OBJECT_H