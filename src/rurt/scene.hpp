/* scene.hpp
 *
 * contains the definition of the scene class, which represents an entire
 * 3d scene for which to render 
 */

#ifndef RURT_SCENE_H
#define RURT_SCENE_H

#include "ray.hpp"
#include "object.hpp"

#include "quickmath.hpp"
using namespace qm;

//-------------------------------------------//

namespace rurt
{

class Scene
{
public:
	Scene(const std::vector<std::pair<std::shared_ptr<const Object>, mat4>>& objects);

	vec3 intersect(const Ray& ray) const;

private:
	vec3 sky_color(const Ray& ray) const;

	struct ObjectRef
	{
		std::shared_ptr<const Object> object;
		mat4 transform;
		mat4 invTransform;
		mat4 invTransformNoTranslate;
	};
	std::vector<ObjectRef> m_objects;
};

}; //namespace rurt

#endif //#ifndef RURT_SCENE_H