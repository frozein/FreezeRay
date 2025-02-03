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

	m_triDistribution = std::make_unique<Distribution<uint32_t>>(pmf, m_area);
}

vec3 LightArea::sample_li(const IntersectionInfo& hitInfo, const vec3& u, vec3& wiWorld, VisibilityTestInfo& vis, float& pdf) const
{
	vec3 pos = sample_mesh_area(u);
	vec3 toLight = pos - hitInfo.worldPos;

	wiWorld = normalize(toLight);
	pdf = 1.0f / m_area;
	vis.infinite = false;
	vis.endPos = pos;

	return m_intensity;
}

vec3 LightArea::le(const IntersectionInfo& hitInfo, const vec3& w) const
{
	return m_intensity;
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

vec3 LightArea::sample_mesh_area(const vec3& u) const
{
	//get triangle:
	//---------------
	uint32_t triIdx = m_triDistribution->sample(u.x);
    
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

    return b0 * v0 + b1 * v1 + b2 * v2;
}

}; //namespace fr