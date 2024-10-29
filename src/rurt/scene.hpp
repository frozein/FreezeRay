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
	Scene(std::vector<std::pair<std::shared_ptr<Object>, mat4>> objects);

	vec3 intersect(const Ray& ray);

private:
	vec3 sky_color(const Ray& ray);

	struct ObjectRef
	{
		std::shared_ptr<Object> object;
		mat4 transform;
		mat4 invTransform;
		mat4 invTransformNoTranslate;
	};
	std::vector<ObjectRef> m_objects;
};

}; //namespace rurt

#endif //#ifndef RURT_SCENE_H