/* fr_object.hpp
 *
 * contains the definition of the object class, which represents
 * a single object (collection of meshes and materials)
 */

#ifndef FR_OBJECT_H
#define FR_OBJECT_H

#include "fr_mesh.hpp"
#include "fr_material.hpp"

//-------------------------------------------//

namespace fr
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
	Object(const std::shared_ptr<const Mesh>& mesh, const std::shared_ptr<const Material>& material);
	Object(const std::vector<ObjectComponent>& components);

	bool intersect(const Ray& ray, float& t, vec2& uv, vec3& normal, IntersectionInfo::Derivatives& derivs, std::shared_ptr<const Material>& material) const;

	static std::shared_ptr<const Object> from_obj(const std::string& objPath, const std::string& mtlPath, bool opacityIsMask = true);

private:
	std::vector<ObjectComponent> m_components;
};

}; //namespace fr

#endif //#ifndef FR_OBJECT_H