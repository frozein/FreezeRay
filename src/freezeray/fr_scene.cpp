#include "freezeray/fr_scene.hpp"
#include "freezeray/fr_globals.hpp"

//-------------------------------------------//

namespace fr
{

class MaterialNoScattering : public Material
{
public:
	MaterialNoScattering() : Material("no scattering")
	{

	}

	std::shared_ptr<BSDF> get_bsdf(const IntersectionInfo& hitInfo) const
	{
		return std::make_shared<BSDF>(hitInfo.worldNormal);
	}
};

//-------------------------------------------//

Scene::Scene(const std::vector<ObjectReference>& objects, std::vector<std::unique_ptr<Light>>& lights)
{
	//add objects:
	//---------------
	m_worldBounds.min = vec3( INFINITY);
	m_worldBounds.max = vec3(-INFINITY);

	for(uint32_t i = 0; i < objects.size(); i++)
	{
		bound3 objBounds = transform_bounds(objects[i].object->get_bounds(), objects[i].transform);
		m_worldBounds.min = min(m_worldBounds.min, objBounds.min);
		m_worldBounds.max = max(m_worldBounds.max, objBounds.max);

		add_object_reference(objects[i].object, nullptr, objects[i].transform);
	}

	//add lights:
	//---------------
	std::shared_ptr<const Material> lightMaterial = std::make_shared<MaterialNoScattering>();
	for(uint32_t i = 0; i < lights.size(); i++)
	{
		std::shared_ptr<const Light> light = std::move(lights[i]);

		mat4 lightTransform;
		std::shared_ptr<const Mesh> lightMesh = light->get_mesh(lightTransform);

		if(lightMesh != nullptr)
		{
			std::vector<ObjectComponent> componentList = { { lightMesh, lightMaterial } };
			std::shared_ptr<const Object> lightObject = std::make_shared<Object>(componentList);

			bound3 lightBounds = transform_bounds(lightMesh->get_bounds(), lightTransform);
			m_worldBounds.min = min(m_worldBounds.min, lightBounds.min);
			m_worldBounds.max = max(m_worldBounds.max, lightBounds.max);

			add_object_reference(lightObject, light, lightTransform);
		}

		if(light->is_infinite())
			m_infiniteLights.push_back(light);

		m_lights.push_back(light);
	}

	//preprocess lights:
	//---------------
	for(uint32_t i = 0; i < m_lights.size(); i++)
		std::const_pointer_cast<Light>(m_lights[i])->preprocess(std::shared_ptr<Scene>(this, [](Scene*){}));
}

bool Scene::intersect(const Ray& worldRay, IntersectionInfo& hitInfo) const
{
	float minT = INFINITY;

	vec3 minWorldHitPos;
	vec3 minObjectHitPos;

	vec2 minUV;
	vec3 minWorldNormal;
	vec3 minObjectNormal;
	IntersectionInfo::Derivatives minDerivs;
	std::shared_ptr<const Material> minMaterial;
	std::shared_ptr<const Light> minLight;

	bool hit = false;

	for(uint32_t i = 0; i < m_objects.size(); i++)
	{
		Ray objectRay = worldRay.transformed(m_objects[i].invTransform, m_objects[i].invTransformNoTranslate);

		float t;
		vec2 uv;
		vec3 objectNormal;
		IntersectionInfo::Derivatives derivs;
		std::shared_ptr<const Material> material;
		if(m_objects[i].object->intersect(objectRay, t, uv, objectNormal, derivs, material))
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
				minDerivs = derivs;
				minMaterial = material;
				minLight = m_objects[i].light;
			}
		}
	}

	if(hit)
	{
		hitInfo.bsdf = nullptr;
		hitInfo.light = minLight;
		hitInfo.worldPos = minWorldHitPos;
		hitInfo.objectPos = minObjectHitPos;
		hitInfo.worldNormal = normalize(minWorldNormal);
		hitInfo.objectNormal = normalize(minObjectNormal);
		hitInfo.uv = minUV;

		hitInfo.bsdf = minMaterial->get_bsdf(hitInfo);
	}
	else
	{
		hitInfo.bsdf = nullptr;
		hitInfo.light = nullptr;
		hitInfo.worldPos = vec3(0.0f);
		hitInfo.objectPos = vec3(0.0f);
		hitInfo.worldNormal = vec3(0.0f);
		hitInfo.objectNormal = vec3(0.0f);
		hitInfo.uv = vec2(0.0f);
	}

	hitInfo.derivatives = minDerivs;

	return hit;
}

const std::vector<std::shared_ptr<const Light>>& Scene::get_lights() const
{
	return m_lights;
}

const std::vector<std::shared_ptr<const Light>>& Scene::get_infinite_lights() const
{
	return m_infiniteLights;
}

bound3 Scene::get_world_bounds() const
{
	return m_worldBounds;
}

float Scene::get_world_radius() const
{
	return std::max(length(m_worldBounds.min), length(m_worldBounds.max));
}

void Scene::add_object_reference(const std::shared_ptr<const Object>& object, const std::shared_ptr<const Light>& light, const mat4& transform)
{
	ObjectReferenceFull ref;
	ref.object = object;
	ref.light = light;

	ref.transform = transform;
	ref.transformNoTranslate = transform;
	ref.transformNoTranslate.m[3][0] = 0.0f;
	ref.transformNoTranslate.m[3][1] = 0.0f;
	ref.transformNoTranslate.m[3][2] = 0.0f;

	ref.invTransform = inverse(transform);
	ref.invTransformNoTranslate = ref.invTransform;
	ref.invTransformNoTranslate.m[3][0] = 0.0f;
	ref.invTransformNoTranslate.m[3][1] = 0.0f;
	ref.invTransformNoTranslate.m[3][2] = 0.0f;

	m_objects.push_back(ref);
}

bound3 Scene::transform_bounds(const bound3& bounds, const mat4& transform)
{
	vec3 points[8];
	points[0] = vec3(bounds.min.x, bounds.min.y, bounds.min.z);
	points[1] = vec3(bounds.max.x, bounds.min.y, bounds.min.z);
	points[2] = vec3(bounds.min.x, bounds.max.y, bounds.min.z);
	points[3] = vec3(bounds.max.x, bounds.max.y, bounds.min.z);
	points[4] = vec3(bounds.min.x, bounds.min.y, bounds.max.z);
	points[5] = vec3(bounds.max.x, bounds.min.y, bounds.max.z);
	points[6] = vec3(bounds.min.x, bounds.max.y, bounds.max.z);
	points[7] = vec3(bounds.max.x, bounds.max.y, bounds.max.z);

	for(uint32_t i = 0; i < 8; i++)
		points[i] = (transform * vec4(points[i], 1.0f)).xyz();

	bound3 newBounds;
	newBounds.min = points[0];
	newBounds.max = points[0];

	for(uint32_t i = 1; i < 8; i++)
	{
		newBounds.min = min(newBounds.min, points[i]);
		newBounds.max = max(newBounds.max, points[i]);
	}

	return newBounds;
}

}; //namespace fr