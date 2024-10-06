#include "scene.hpp"

#define EPSILON 0.0001f

//-------------------------------------------//

namespace rurt
{

Scene::Scene()
{
	//TODO

	m_verts[0] = vec3(-0.5f, -0.5f, -1.0f);
	m_verts[1] = vec3(0.5f, -0.5f, -1.0f);
	m_verts[2] = vec3(0.0f, 0.5f, -1.0f);

	m_cullBackface = true;
}

Scene::~Scene()
{
	//TODO
}

vec3 Scene::intersect(const Ray& ray)
{
	float t;
	float u, v;
	bool hit = intersect_triangle(ray, m_verts, t, u, v);

	if(hit)
	{
		float w = 1.0f - u - v;

		return vec3(w, u, v);
	}
	else
		return sky_color(ray);
}

//-------------------------------------------//

bool Scene::intersect_triangle(const Ray& ray, vec3* verts, float& t, float& u, float& v)
{
	//TODO: allow specification of handedness and winding order

	vec3 v0v1 = verts[1] - verts[0];
	vec3 v0v2 = verts[2] - verts[0];
	vec3 pvec = cross(ray.direction(), v0v2);
	float det = dot(v0v1, pvec);

	if(m_cullBackface)
	{
		if(det < EPSILON)
			return false;
	}
	else
	{
		if(abs(det) < EPSILON)
			return false;
	}

	float invDet = 1.0f / det;

	vec3 tvec = ray.origin() - verts[0];
	u = dot(tvec, pvec) * invDet;
	if(u < 0.0f || u > 1.0f)
		return false;

	vec3 qvec = cross(tvec, v0v1);
	v = dot(ray.direction(), qvec) * invDet;
	if(v < 0.0f || u + v > 1.0f)
		return false;

	t = dot(v0v2, qvec) * invDet;
}

vec3 Scene::sky_color(const Ray& ray)
{
	float skyPos = ray.direction().y * 0.5f + 0.5f;
	return (1.0f - skyPos) * vec3(0.71f, 0.85f, 0.90f) + skyPos * vec3(0.00f, 0.45f, 0.74f);
}

}; //namespace rurt