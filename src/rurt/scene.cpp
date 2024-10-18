#include "scene.hpp"

#define EPSILON 0.0001f

//-------------------------------------------//

namespace rurt
{

Scene::Scene(std::shared_ptr<Mesh> mesh) :
	m_mesh(mesh)
{
	//TODO
}

Scene::~Scene()
{
	//TODO
}

vec3 Scene::intersect(const Ray& ray)
{
	float t;
	vec2 uv;
	vec3 normal;
	bool hit = m_mesh->intersect(ray, t, uv, normal);

	if(hit)
		return normalize(vec3(abs(normal.x), abs(normal.y), abs(normal.z)));
	else
		return sky_color(ray);
}

//-------------------------------------------//

vec3 Scene::sky_color(const Ray& ray)
{
	float skyPos = ray.direction().y * 0.5f + 0.5f;
	return (1.0f - skyPos) * vec3(0.71f, 0.85f, 0.90f) + skyPos * vec3(0.00f, 0.45f, 0.74f);
}

}; //namespace rurt