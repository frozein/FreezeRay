#include "mesh.hpp"

#define CULL_BACKFACE 1
#define EPSILON 0.0001f

//-------------------------------------------//

namespace rurt
{

Mesh::Mesh(uint32_t vertexAttribs, uint32_t numFaces, std::shared_ptr<uint32_t[]> faceIndices, 
           std::shared_ptr<uint32_t[]> vertIndices, std::shared_ptr<float[]> verts, std::string material) :
	m_vertexAttribs(vertexAttribs),
	m_verts(verts),
	m_material(material)
{
	if((vertexAttribs & VERTEX_ATTRIB_POSITION) == 0)
	{
		//TODO: proper error logging/handling
		std::cout << "ERROR: vertices MUST contain the POSITION attribute" << std::endl;
		return;
	}

	uint32_t numTris = 0;
	for(uint32_t i = 0; i < numFaces; i++)
		numTris += faceIndices[i] - 2;

	m_numTris = numTris;
	m_indices = std::shared_ptr<uint32_t[]>(new uint32_t [numTris * 3]);

	uint32_t k = 0, l = 0;
	for(uint32_t i = 0; i < numFaces; i++)
	{
		for(uint32_t j = 0; j < faceIndices[i] - 2; j++)
		{
			m_indices[l + 0] = vertIndices[k + 0];
			m_indices[l + 1] = vertIndices[k + j + 1];
			m_indices[l + 2] = vertIndices[k + j + 2];

			l += 3;
		}

		k += faceIndices[i];
	}
}

Mesh::Mesh(uint32_t vertexAttribs, uint32_t numTris, std::shared_ptr<uint32_t[]> indices, 
           std::shared_ptr<float[]> verts, std::string material) :
	m_vertexAttribs(vertexAttribs),
	m_numTris(numTris),
	m_indices(indices),
	m_verts(verts),
	m_material(material)
{
	if((vertexAttribs & VERTEX_ATTRIB_POSITION) == 0)
	{
		//TODO: proper error logging/handling
		std::cout << "ERROR: vertices MUST contain the POSITION attribute" << std::endl;
		return;
	}
}

std::string Mesh::get_material()
{
	return m_material;
}

bool Mesh::intersect(const Ray& ray, float& minT, vec2& uv, vec3& normal)
{
	float* verts = m_verts.get();

	uint32_t stride = 0;
	uint32_t uvOffset = 0;
	uint32_t normalOffset = 0;

	if((m_vertexAttribs & VERTEX_ATTRIB_POSITION) != 0)
	{
		stride += 3;
		uvOffset += 3;
		normalOffset += 3;
	}
	if((m_vertexAttribs & VERTEX_ATTRIB_UV) != 0)
	{
		stride += 2;
		normalOffset += 2;
	}
	if((m_vertexAttribs & VERTEX_ATTRIB_NORMAL) != 0)
		stride += 3;
	
	bool hit = false;
	minT = INFINITY;
	for(uint32_t i = 0; i < m_numTris; i++)
	{
		uint32_t triIdx = i * 3;
		uint32_t idx0 = m_indices[triIdx + 0] * stride;
		uint32_t idx1 = m_indices[triIdx + 1] * stride;
		uint32_t idx2 = m_indices[triIdx + 2] * stride;
	
		const vec3& v0 = *reinterpret_cast<const vec3*>(&verts[idx0]);
		const vec3& v1 = *reinterpret_cast<const vec3*>(&verts[idx1]);
		const vec3& v2 = *reinterpret_cast<const vec3*>(&verts[idx2]);

		float t;
		float u, v;
		if(intersect_triangle(ray, v0, v1, v2, t, u, v) && t < minT)
		{
			hit |= true;
			minT = t;

			float w = 1.0f - u - v;

			if((m_vertexAttribs & VERTEX_ATTRIB_UV) != 0)
			{
				const vec2& uv0 = *reinterpret_cast<const vec2*>(&verts[idx0 + uvOffset]);
				const vec2& uv1 = *reinterpret_cast<const vec2*>(&verts[idx1 + uvOffset]);
				const vec2& uv2 = *reinterpret_cast<const vec2*>(&verts[idx2 + uvOffset]);

				uv = uv0 * w + uv1 * u + uv2 * v;
			}
			else
				uv = vec2(0.0f);

			if((m_vertexAttribs & VERTEX_ATTRIB_NORMAL) != 0)
			{
				const vec3& normal0 = *reinterpret_cast<const vec3*>(&verts[idx0 + normalOffset]);
				const vec3& normal1 = *reinterpret_cast<const vec3*>(&verts[idx1 + normalOffset]);
				const vec3& normal2 = *reinterpret_cast<const vec3*>(&verts[idx2 + normalOffset]);

				normal = normal0 * w + normal1 * u + normal2 * v;
			}
			else
				normal = cross(v1 - v0, v2 - v0);
		}
	}

	return hit;
}

//-------------------------------------------//

bool Mesh::intersect_triangle(const Ray& ray, const vec3& v0, const vec3& v1, const vec3& v2, float& t, float& u, float& v)
{
	//TODO: allow specification of handedness and winding order

	vec3 v0v1 = v1 - v0;
	vec3 v0v2 = v2 - v0;
	vec3 pvec = cross(ray.direction(), v0v2);
	float det = dot(v0v1, pvec);

#if CULL_BACKFACE
	{
		if(det < EPSILON)
			return false;
	}
#else
	{
		if(abs(det) < EPSILON)
			return false;
	}
#endif

	float invDet = 1.0f / det;

	vec3 tvec = ray.origin() - v0;
	u = dot(tvec, pvec) * invDet;
	if(u < 0.0f || u > 1.0f)
		return false;

	vec3 qvec = cross(tvec, v0v1);
	v = dot(ray.direction(), qvec) * invDet;
	if(v < 0.0f || u + v > 1.0f)
		return false;

	t = dot(v0v2, qvec) * invDet;
	return true;
}

}; //namespace rurt