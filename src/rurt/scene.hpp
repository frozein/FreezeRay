/* scene.hpp
 *
 * contains the definition of the scene class, which represents an entire
 * 3d scene for which to render 
 */

#ifndef RURT_SCENE_H
#define RURT_SCENE_H

#include "ray.hpp"

#include "quickmath.hpp"
using namespace qm;

//-------------------------------------------//

namespace rurt
{

class Scene
{
public:
	Scene();
	~Scene();

	vec3 intersect(const Ray& ray);

private:
	bool intersect_triangle(const Ray& ray, vec3* verts, float& t, float& u, float& v);
	vec3 sky_color(const Ray& ray);

	vec3 m_verts[3]; //TEMP: just contains a single triangle for testing for now

	//PARAMS (TODO: wrap into object):
	bool m_cullBackface;
};

}; //namespace rurt

#endif //#ifndef RURT_SCENE_H