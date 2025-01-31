/* fr_scene.hpp
 *
 * contains the definition of the scene class, which represents an entire
 * 3d scene for which to render 
 */

#ifndef FR_SCENE_H
#define FR_SCENE_H

#include "fr_ray.hpp"
#include "fr_object.hpp"
#include "fr_light.hpp"
#include "fr_raycast_info.hpp"

#include "quickmath.hpp"
using namespace qm;

//-------------------------------------------//

namespace fr
{

struct ObjectReference
{
	std::shared_ptr<const Object> object;
	mat4 transform;
};

class Scene
{
public:
	Scene(const std::vector<ObjectReference>& objects, const std::vector<std::shared_ptr<const Light>>& lights);

	bool intersect(const Ray& ray, IntersectionInfo& info) const;

	const std::vector<std::shared_ptr<const Light>>& get_lights() const;
	const std::vector<std::shared_ptr<const Light>>& get_infinite_lights() const;

private:
	struct ObjectReferenceFull
	{
		std::shared_ptr<const Object> object;
		std::shared_ptr<const Light> light;

		mat4 transform;
		mat4 transformNoTranslate;
		mat4 invTransform;
		mat4 invTransformNoTranslate;
	};
	std::vector<ObjectReferenceFull> m_objects;

	std::vector<std::shared_ptr<const Light>> m_lights;
	std::vector<std::shared_ptr<const Light>> m_infiniteLights;

	void add_object_reference(const std::shared_ptr<const Object>& object, const std::shared_ptr<const Light>& light, const mat4& transform);
};

}; //namespace fr

#endif //#ifndef FR_SCENE_H