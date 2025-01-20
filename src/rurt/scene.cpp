#include "rurt/scene.hpp"
#include "rurt/globals.hpp"

//-------------------------------------------//

namespace rurt
{

class MaterialNoScattering : public Material
{
public:
	MaterialNoScattering() : Material("no scattering", true, BXDFType::REFLECTION) 
	{

	}

	vec3 bsdf_f(const IntersectionInfo& hitInfo, const vec3& wiWorld, const vec3& woWorld) const override 
	{ 
		return vec3(0.0f); 
	}

	vec3 bsdf_sample_f(const IntersectionInfo& hitInfo, vec3& wiWorld, const vec3& woWorld, const vec2& u, float& pdf) const override 
	{
		wiWorld = hitInfo.worldNormal;
		pdf = 0.0f;
		return vec3(0.0f);
	}

	float bsdf_pdf(const IntersectionInfo& hitInfo, const vec3& wiWorld, const vec3& woWorld) const override
	{
		return 0.0f;
	}
};

//-------------------------------------------//

Scene::Scene(const std::vector<ObjectReference>& objects, const std::vector<std::shared_ptr<const Light>>& lights) :
	m_lights(lights)
{
	for(uint32_t i = 0; i < objects.size(); i++)
		add_object_reference(objects[i].object, nullptr, objects[i].transform);

	std::shared_ptr<const Material> lightMaterial = std::make_shared<MaterialNoScattering>();
	for(uint32_t i = 0; i < lights.size(); i++)
	{
		mat4 lightTransform;
		std::shared_ptr<const Mesh> lightMesh = lights[i]->get_mesh(lightTransform);

		if(lightMesh == nullptr)
			continue;

		std::vector<ObjectComponent> componentList = { { lightMesh, lightMaterial } };
		std::shared_ptr<const Object> lightObject = std::make_shared<Object>(componentList);

		add_object_reference(lightObject, lights[i], lightTransform);
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
		hitInfo.material = minMaterial;
		hitInfo.light = minLight;
		hitInfo.worldPos = minWorldHitPos;
		hitInfo.objectPos = minObjectHitPos;
		hitInfo.worldNormal = normalize(minWorldNormal);
		hitInfo.objectNormal = normalize(minObjectNormal);
		hitInfo.uv = minUV;
	}
	else
	{
		hitInfo.material = nullptr;
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

}; //namespace rurt