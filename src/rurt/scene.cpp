#include "rurt/scene.hpp"
#include "rurt/globals.hpp"

//-------------------------------------------//

namespace rurt
{

Scene::Scene(const std::vector<std::pair<std::shared_ptr<const Object>, mat4>>& objects)
{
	for(uint32_t i = 0; i < objects.size(); i++)
	{
		ObjectRef ref;
		ref.object = objects[i].first;

		ref.transform = objects[i].second;
		ref.transformNoTranslate = objects[i].second;
		ref.transformNoTranslate.m[3][0] = 0.0f;
		ref.transformNoTranslate.m[3][1] = 0.0f;
		ref.transformNoTranslate.m[3][2] = 0.0f;

		ref.invTransform = inverse(objects[i].second);
		ref.invTransformNoTranslate = ref.invTransform;
		ref.invTransformNoTranslate.m[3][0] = 0.0f;
		ref.invTransformNoTranslate.m[3][1] = 0.0f;
		ref.invTransformNoTranslate.m[3][2] = 0.0f;

		m_objects.push_back(ref);
	}
}

RaycastInfo Scene::intersect(const Ray& worldRay, std::shared_ptr<const Material>& hitMaterial) const
{
	float minT = INFINITY;

	vec3 minWorldHitPos;
	vec3 minObjectHitPos;

	vec2 minUV;
	vec3 minWorldNormal;
	vec3 minObjectNormal;
	std::shared_ptr<const Material> minMaterial;
	
	bool hit = false;

	for(uint32_t i = 0; i < m_objects.size(); i++)
	{
		Ray objectRay = worldRay.transformed(m_objects[i].invTransform, m_objects[i].invTransformNoTranslate);

		float t;
		vec2 uv;
		vec3 objectNormal;
		std::shared_ptr<const Material> material;
		if(m_objects[i].object->intersect(objectRay, t, uv, objectNormal, material))
		{
			hit |= true;

			vec3 objectHitPos = objectRay.at(t);
			vec3 worldHitPos = (m_objects[i].transform * vec4(objectHitPos, 1.0)).xyz();
			t = distance(worldRay.origin(), worldHitPos);

			if(t < minT)
			{
				minT = t;

				minWorldHitPos = worldHitPos;
				minObjectHitPos = objectHitPos;

				minUV = uv;
				minWorldNormal = (m_objects[i].transformNoTranslate * vec4(objectNormal, 1.0)).xyz();
				minObjectNormal = objectNormal;
				minMaterial = material;
			}
		}
	}

	RaycastInfo retval = {};
	if(hit)
	{
		hitMaterial = minMaterial;

		retval.hitInfo.worldPos = minWorldHitPos;
		retval.hitInfo.objectPos = minObjectHitPos;
		retval.hitInfo.worldNormal = normalize(minWorldNormal);
		retval.hitInfo.objectNormal = normalize(minObjectNormal);
		retval.hitInfo.uv = minUV;
	}
	else
	{
		hitMaterial = nullptr;

		retval.missInfo.skyColor = sky_color(worldRay);
		retval.missInfo.skyEmission = sky_emission(worldRay);
	}

	return retval;
}

//-------------------------------------------//

vec3 Scene::sky_color(const Ray& ray) const
{
	//temp implementation

	float skyPos = ray.direction().y * 0.5f + 0.5f;
	return (1.0f - skyPos) * vec3(0.71f, 0.85f, 0.90f) + skyPos * vec3(0.00f, 0.45f, 0.74f);
}

vec3 Scene::sky_emission(const Ray& ray) const
{
	//temp implementation
	
	return vec3(std::max(ray.direction().y, 0.0f));
}

}; //namespace rurt