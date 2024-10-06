/* scene.hpp
 *
 * contains the definition of the scene class, which represents an entire
 * 3d scene for which to render 
 */

#ifndef RURT_SCENE_H
#define RURT_SCENE_H

#include "ray.hpp"
#include "mesh.hpp"

#include "quickmath.hpp"
using namespace qm;

//-------------------------------------------//

namespace rurt
{

class Scene
{
public:
	Scene(std::shared_ptr<Mesh> mesh);
	~Scene();

	vec3 intersect(const Ray& ray);

private:
	vec3 sky_color(const Ray& ray);

	std::shared_ptr<Mesh> m_mesh; //TEMP: just contains a single mesh for testing for now
};

}; //namespace rurt

#endif //#ifndef RURT_SCENE_H