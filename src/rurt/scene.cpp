#include "rurt/scene.hpp"
#include "rurt/globals.hpp"

//-------------------------------------------//

namespace rurt
{

Scene::Scene(const std::vector<ObjectReference>& objects, const std::vector<std::shared_ptr<const Light>>& lights) :
	m_lights(lights)
{
	for(uint32_t i = 0; i < objects.size(); i++)
	{
		ObjectReferenceFull ref;
		ref.object = objects[i].object;

		ref.transform = objects[i].transform;
		ref.transformNoTranslate = objects[i].transform;
		ref.transformNoTranslate.m[3][0] = 0.0f;
		ref.transformNoTranslate.m[3][1] = 0.0f;
		ref.transformNoTranslate.m[3][2] = 0.0f;

		ref.invTransform = inverse(objects[i].transform);
		ref.invTransformNoTranslate = ref.invTransform;
		ref.invTransformNoTranslate.m[3][0] = 0.0f;
		ref.invTransformNoTranslate.m[3][1] = 0.0f;
		ref.invTransformNoTranslate.m[3][2] = 0.0f;

		m_objects.push_back(ref);
	}
}

bool Scene::intersect(const Ray& worldRay, IntersectionInfo& hitInfo) const
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

	if(hit)
	{
		hitInfo.material = minMaterial;
		hitInfo.worldPos = minWorldHitPos;
		hitInfo.objectPos = minObjectHitPos;
		hitInfo.worldNormal = normalize(minWorldNormal);
		hitInfo.objectNormal = normalize(minObjectNormal);
		hitInfo.uv = minUV;
	}
	else
	{
		hitInfo.material = nullptr;
		hitInfo.worldPos = vec3(0.0f);
		hitInfo.objectPos = vec3(0.0f);
		hitInfo.worldNormal = vec3(0.0f);
		hitInfo.objectNormal = vec3(0.0f);
		hitInfo.uv = vec2(0.0f);
	}

	return hit;
}

const std::vector<std::shared_ptr<const Light>>& Scene::get_lights() const
{
	return m_lights;
}

}; //namespace rurt