#include "freezeray/light/fr_light_area.hpp"
#include "freezeray/fr_globals.hpp"

//-------------------------------------------//

namespace fr
{

LightArea::LightArea(const std::shared_ptr<const Mesh>& mesh, const mat4& transform, const vec3& intensity) :
	Light(false, false), m_mesh(mesh), m_transform(transform), m_intensity(intensity)
{
	//validate:
	//---------------
	if(!m_mesh)
		throw std::invalid_argument("mesh must not be NULL");

	//compute inv transforms:
	//---------------
	m_invTransform = inverse(transform);

	m_invTransformNoTranslate = m_invTransform;
	m_invTransformNoTranslate.m[3][0] = 0.0f;
	m_invTransformNoTranslate.m[3][1] = 0.0f;
	m_invTransformNoTranslate.m[3][2] = 0.0f;	

	//generate triangle distribution:
	//---------------
	const uint32_t numTris = m_mesh->get_num_tris();
	
	m_area = 0.0f;
	std::vector<std::pair<uint32_t, float>> pmf(numTris);

	for(uint32_t i = 0; i < numTris; i++) 
	{
		uint32_t idx0;
		uint32_t idx1;
		uint32_t idx2;
		m_mesh->get_tri_indices(i, idx0, idx1, idx2);

		vec3 v0 = m_mesh->get_vert_pos_at(idx0);
		vec3 v1 = m_mesh->get_vert_pos_at(idx1);
		vec3 v2 = m_mesh->get_vert_pos_at(idx2);

		v0 = (m_transform * vec4(v0, 1.0f)).xyz();
		v1 = (m_transform * vec4(v1, 1.0f)).xyz();
		v2 = (m_transform * vec4(v2, 1.0f)).xyz();

		float triArea = length(cross(v1 - v0, v2 - v0)) * 0.5f;
		pmf[i] = { i, triArea };
		m_area += triArea;
	}

	m_triDistribution = std::make_unique<DistributionDiscrete<uint32_t>>(pmf);
}

vec3 LightArea::sample_li(const IntersectionInfo& hitInfo, const vec3& u, vec3& wiWorld, VisibilityTestInfo& vis, float& pdf) const
{
	vec3 normal;
	vec3 pos = sample_mesh_area(u, pdf, normal);
	vec3 toLight = pos - hitInfo.pos;

	wiWorld = normalize(toLight);
	pdf *= dot(toLight, toLight) / std::abs(dot(-1.0f * wiWorld, hitInfo.shadingNormal));
	vis.startPos = hitInfo.pos;
	vis.endPos = pos;

	return m_intensity;
}

float LightArea::pdf_li(const IntersectionInfo& hitInfo, const vec3& w) const
{
	vec3 pos = hitInfo.pos + FR_EPSILON * normalize(w);
	Ray ray(pos, w);

	float t;
	vec2 uv;
	vec3 normal;
	IntersectionInfo::Derivatives derivs;
	if(!m_mesh->intersect(ray.transformed(m_invTransform, m_invTransformNoTranslate), nullptr, t, uv, normal, derivs))
		return 0.0f;

	return t * t / (std::abs(dot(normal, -1.0f * w)) * m_area);
}

vec3 LightArea::le(const IntersectionInfo& hitInfo, const vec3& w) const
{
	return m_intensity;
}

vec3 LightArea::sample_le(const vec3& u1, const vec3& u2, Ray& ray, vec3& normal, float& pdfPos, float& pdfDir) const
{
	//sample position:
	//---------------
	vec3 objNormal;
	vec3 pos = sample_mesh_area(u1, pdfPos, objNormal);
	normal = normalize((m_transform * vec4(objNormal, 0.0f)).xyz());

	//sample direction:
	//---------------
	float r = std::sqrtf(u2.x);
	float theta = FR_2_PI * u2.y;

	vec2 xz = vec2(r * std::cos(theta), r * std::sin(theta));
	float y = std::sqrtf(1.0f - xz.x * xz.x - xz.y * xz.y);
	vec3 dir = vec3(xz.x, y, xz.y);

	vec3 v1, v2;
	get_orthogonal(normal, v1, v2);
	dir = v1 * dir.x + normal * dir.y + v2 * dir.z;

	pdfDir = std::abs(dot(normal, dir)) * FR_INV_PI;

	//return:
	//---------------
	ray = Ray(pos, dir);

	return m_intensity;
}

void LightArea::pdf_le(const Ray& ray, const vec3& normal, float& pdfPos, float& pdfDir) const
{
	pdfPos = 1.0f / m_area;
	pdfDir = std::abs(dot(ray.direction(), normal)) * FR_INV_PI;
}

vec3 LightArea::power() const
{
	return m_intensity * m_area * FR_2_PI;
}

std::shared_ptr<const Mesh> LightArea::get_mesh(mat4& transform) const
{
	transform = m_transform;
	return m_mesh;
}

vec3 LightArea::sample_mesh_area(const vec3& u, float& pdf, vec3& objNormal) const
{
	//get triangle:
	//---------------
	uint32_t triIdx = m_triDistribution->sample(u.x, pdf);
    
	uint32_t idx0;
    uint32_t idx1;
    uint32_t idx2;
	m_mesh->get_tri_indices(triIdx, idx0, idx1, idx2);

    vec3 v0 = m_mesh->get_vert_pos_at(idx0);
    vec3 v1 = m_mesh->get_vert_pos_at(idx1);
    vec3 v2 = m_mesh->get_vert_pos_at(idx2);

	v0 = (m_transform * vec4(v0, 1.0f)).xyz();
	v1 = (m_transform * vec4(v1, 1.0f)).xyz();
	v2 = (m_transform * vec4(v2, 1.0f)).xyz();

	//choose barycentric coordinates:
	//---------------
    float sqrtR1 = sqrt(u.y);

    float b0 = 1.0f - sqrtR1;
	float b1 = u.z * sqrtR1;
    float b2 = 1.0f - b0 - b1;

	//update pdf to account for tri area, return:
	//---------------
	objNormal = cross(v1 - v0, v2 - v0);

	float triArea = length(objNormal) * 0.5f;
	pdf /= triArea;

    return b0 * v0 + b1 * v1 + b2 * v2;
}

}; //namespace fr