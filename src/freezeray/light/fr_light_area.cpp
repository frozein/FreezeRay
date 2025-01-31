#include "freezeray/light/fr_light_area.hpp"
#include "freezeray/fr_globals.hpp"

//-------------------------------------------//

namespace fr
{

LightArea::LightArea(const std::shared_ptr<const Mesh>& mesh, const mat4& transform, const vec3& intensity) :
	Light(false, false), m_mesh(mesh), m_transform(transform), m_intensity(intensity)
{
	if(!m_mesh)
		throw std::invalid_argument("no mesh provided to LightArea");

	generate_alias_table();
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

void LightArea::generate_alias_table()
{
	//calculate area of each triangle:
	//---------------
	const uint32_t numTris = m_mesh->get_num_tris();
	
	m_area = 0.0f;
	std::vector<float> areas(numTris);

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
		areas[i] = triArea;
		m_area += triArea;
	}

	//generate scaled probabilities:
	//---------------
	std::vector<float> scaledProbs(numTris);
	for (uint32_t i = 0; i < numTris; i++)
		scaledProbs[i] = (areas[i] / m_area) * numTris;

	//create small/large worklists:
	//---------------
	std::vector<uint32_t> small;
	std::vector<uint32_t> large;

	for (uint32_t i = 0; i < numTris; i++) 
	{
		if (scaledProbs[i] < 1.0f)
			small.push_back(i);
		else
			large.push_back(i);
	}

	//process small/large worklists:
	//---------------
	m_acceptanceTable.resize(numTris);
	m_aliasTable.resize(numTris);

	while(!small.empty() && !large.empty()) 
	{
		uint32_t s = small.back(); 
		uint32_t l = large.back(); 
		
		small.pop_back();
		large.pop_back();

		m_acceptanceTable[s] = scaledProbs[s];
		m_aliasTable[s] = l;

		scaledProbs[l] = (scaledProbs[l] + scaledProbs[s]) - 1.0f;

		if (scaledProbs[l] < 1.0f)
			small.push_back(l);
		else
			large.push_back(l);
	}

	for(uint32_t l : large)
		m_acceptanceTable[l] = 1.0f;

	for(uint32_t s : small)
		m_acceptanceTable[s] = 1.0f;
}

vec3 LightArea::sample_mesh_area(const vec3& u) const
{
	//select triangle:
	//---------------
	float triSelect = u.x * m_mesh->get_num_tris();
	uint32_t triIdx = (uint32_t)triSelect;
	if(triIdx == m_mesh->get_num_tris())
		triIdx--;
	
	float acceptanceProb = triSelect - triIdx;
    if(acceptanceProb > m_acceptanceTable[triIdx])
        triIdx = m_aliasTable[triIdx];

	//get triangle vertices:
	//---------------
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