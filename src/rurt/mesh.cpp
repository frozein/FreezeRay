#include "mesh.hpp"

#define CULL_BACKFACE 1
#define EPSILON 0.0001f

//-------------------------------------------//

namespace rurt
{

Mesh::Mesh(uint32_t numFaces, std::shared_ptr<uint32_t[]> faceIndices, std::shared_ptr<uint32_t[]> vertIndices, std::shared_ptr<vec3[]> verts) :
	m_numFaces(numFaces),
	m_faceIndices(faceIndices),
	m_verts(verts)
{
	uint32_t numTris = 0;
	for(uint32_t i = 0; i < m_numFaces; i++)
		numTris += m_faceIndices[i] - 2;

	m_numTris = numTris;
	m_vertIndices = std::unique_ptr<uint32_t[]>(new uint32_t [numTris * 3]);

	uint32_t k = 0, l = 0;
	for(uint32_t i = 0; i < m_numFaces; i++)
	{
		for(uint32_t j = 0; j < m_faceIndices[i] - 2; j++)
		{
			m_vertIndices[l + 0] = vertIndices[k + 0];
			m_vertIndices[l + 1] = vertIndices[k + j + 1];
			m_vertIndices[l + 2] = vertIndices[k + j + 2];

			l += 3;
		}

		k += m_faceIndices[i];
	}
}

Mesh::~Mesh()
{
	//TODO
}

bool Mesh::intersect(const Ray& ray, float& minT, float& minU, float& minV)
{
	bool hit = false;
	minT = INFINITY;
	for(uint32_t i = 0; i < m_numTris; i++)
	{
		uint32_t triIdx = i * 3;
		const vec3& v0 = m_verts[m_vertIndices[triIdx + 0]];
		const vec3& v1 = m_verts[m_vertIndices[triIdx + 1]];
		const vec3& v2 = m_verts[m_vertIndices[triIdx + 2]];

		float t;
		float u, v;
		if(intersect_triangle(ray, v0, v1, v2, t, u, v) && t < minT)
		{
			hit |= true;

			minT = t;
			minU = u;
			minV = v;
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