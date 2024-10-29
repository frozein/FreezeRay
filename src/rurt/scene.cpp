#include "scene.hpp"

#define EPSILON 0.0001f

//-------------------------------------------//

namespace rurt
{

Scene::Scene(std::vector<std::pair<std::shared_ptr<Object>, mat4>> objects)
{
	for(uint32_t i = 0; i < objects.size(); i++)
	{
		ObjectRef ref;
		ref.object = objects[i].first;
		ref.transform = objects[i].second;
		ref.invTransform = inverse(objects[i].second);
		ref.invTransformNoTranslate = ref.invTransform;
		ref.invTransformNoTranslate.m[3][0] = 0.0f;
		ref.invTransformNoTranslate.m[3][1] = 0.0f;
		ref.invTransformNoTranslate.m[3][2] = 0.0f;

		m_objects.push_back(ref);
	}
}

vec3 Scene::intersect(const Ray& ray)
{
	float minT = INFINITY;
	vec2 minUV;
	vec3 minNormal;
	uint32_t minHitIdx;
	bool hit = false;

	for(uint32_t i = 0; i < m_objects.size(); i++)
	{
		Ray objectRay = ray.transformed(m_objects[i].invTransform, m_objects[i].invTransformNoTranslate);

		float t;
		vec2 uv;
		vec3 normal;
		if(m_objects[i].object->intersect(objectRay, t, uv, normal))
		{
			hit |= true;

			vec3 localHitPos = objectRay.at(t);
			vec3 hitPos = (m_objects[i].transform * vec4(localHitPos, 1.0)).xyz();
			t = distance(ray.origin(), hitPos);

			if(t < minT)
			{
				minT = t;
				minUV = uv;
				minNormal = normal;
				minHitIdx = i;
			}
		}
	}

	if(hit)
		return normalize(vec3(abs(minNormal.x), abs(minNormal.y), abs(minNormal.z)));
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