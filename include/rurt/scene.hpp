/* scene.hpp
 *
 * contains the definition of the scene class, which represents an entire
 * 3d scene for which to render 
 */

#ifndef RURT_SCENE_H
#define RURT_SCENE_H

#include "ray.hpp"
#include "object.hpp"
#include "light.hpp"
#include "raycast_info.hpp"

#include "quickmath.hpp"
using namespace qm;

//-------------------------------------------//

namespace rurt
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

private:
	struct ObjectReferenceFull
	{
		std::shared_ptr<const Object> object;
		mat4 transform;
		mat4 transformNoTranslate;
		mat4 invTransform;
		mat4 invTransformNoTranslate;
	};
	std::vector<ObjectReferenceFull> m_objects;

	std::vector<std::shared_ptr<const Light>> m_lights;
};

}; //namespace rurt

#endif //#ifndef RURT_SCENE_H