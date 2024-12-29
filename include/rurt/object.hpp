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

struct ObjectComponent
{
	std::shared_ptr<const Mesh> mesh;
	std::shared_ptr<const Material> material;
};

class Object
{
public:
	Object(const std::vector<std::shared_ptr<const Mesh>>& meshes, const std::vector<std::shared_ptr<const Material>>& materials);
	Object(const std::vector<ObjectComponent>& components);

	bool intersect(const Ray& ray, float& t, vec2& uv, vec3& normal, std::shared_ptr<const Material>& material) const;

private:
	std::vector<ObjectComponent> m_components;
};

}; //namespace rurt

#endif //#ifndef RURT_OBJECT_H