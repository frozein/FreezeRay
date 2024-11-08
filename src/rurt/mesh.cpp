#include "rurt/mesh.hpp"

#define QOBJ_IMPLEMENTATION
#include "rurt/quickobj.h"

#include "rurt/constants.hpp"

#define CULL_BACKFACE 1
#define EPSILON 0.0001f

//-------------------------------------------//

namespace rurt
{

std::unordered_map<std::pair<uint32_t, bool>, std::shared_ptr<const Mesh>, Mesh::HashPair> Mesh::m_unitSpheres = {};
std::shared_ptr<const Mesh> Mesh::m_unitCube = Mesh::gen_unit_cube();
std::shared_ptr<const Mesh> Mesh::m_unitSquare = Mesh::gen_unit_square();

//-------------------------------------------//

Mesh::Mesh(uint32_t vertexAttribs, uint32_t numFaces, std::unique_ptr<uint32_t[]> faceIndices, 
           std::unique_ptr<uint32_t[]> vertIndices, std::unique_ptr<float[]> verts, std::string material,
		   uint32_t vertStride, uint32_t vertPosOffset, uint32_t vertUvOffset, uint32_t vertNormalOffset) :
	m_verts(std::move(verts)),
	m_material(material),
	m_vertAttribs(vertexAttribs),
	m_vertStride(vertStride),
	m_vertPosOffset(vertPosOffset),
	m_vertUvOffset(vertUvOffset),
	m_vertNormalOffset(vertNormalOffset)
{
	//triangulate faces:
	//---------------
	uint32_t numTris = 0;
	for(uint32_t i = 0; i < numFaces; i++)
		numTris += faceIndices[i] - 2;

	m_numTris = numTris;
	m_indices = std::unique_ptr<uint32_t[]>(new uint32_t [numTris * 3]);

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

	//setup strides and offsets if not specified:
	//---------------
	setup_strides_offsets();
}

Mesh::Mesh(uint32_t vertexAttribs, uint32_t numTris, std::unique_ptr<uint32_t[]> indices, 
           std::unique_ptr<float[]> verts, std::string material, uint32_t vertStride,
		   uint32_t vertPosOffset, uint32_t vertUvOffset, uint32_t vertNormalOffset) :
	m_numTris(numTris),
	m_indices(std::move(indices)),
	m_verts(std::move(verts)),
	m_material(material),
	m_vertAttribs(vertexAttribs),
	m_vertStride(vertStride),
	m_vertPosOffset(vertPosOffset),
	m_vertUvOffset(vertUvOffset),
	m_vertNormalOffset(vertNormalOffset)
{
	setup_strides_offsets();
}

const std::string& Mesh::get_material() const
{
	return m_material;
}

void Mesh::set_material(const std::string& material)
{
	m_material = material;
}

bool Mesh::intersect(const Ray& ray, float& minT, vec2& uv, vec3& normal) const
{
	if(!m_valid)
		return false;

	float* verts = m_verts.get();
	
	bool hit = false;
	minT = INFINITY;

	for(uint32_t i = 0; i < m_numTris; i++)
	{
		uint32_t triIdx = i * 3;
		uint32_t idx0 = m_indices[triIdx + 0] * m_vertStride;
		uint32_t idx1 = m_indices[triIdx + 1] * m_vertStride;
		uint32_t idx2 = m_indices[triIdx + 2] * m_vertStride;
	
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

			if((m_vertAttribs & VERTEX_ATTRIB_UV) != 0)
			{
				const vec2& uv0 = *reinterpret_cast<const vec2*>(&verts[idx0 + m_vertUvOffset]);
				const vec2& uv1 = *reinterpret_cast<const vec2*>(&verts[idx1 + m_vertUvOffset]);
				const vec2& uv2 = *reinterpret_cast<const vec2*>(&verts[idx2 + m_vertUvOffset]);

				uv = uv0 * w + uv1 * u + uv2 * v;
			}
			else
				uv = vec2(0.0f);

			if((m_vertAttribs & VERTEX_ATTRIB_NORMAL) != 0)
			{
				const vec3& normal0 = *reinterpret_cast<const vec3*>(&verts[idx0 + m_vertNormalOffset]);
				const vec3& normal1 = *reinterpret_cast<const vec3*>(&verts[idx1 + m_vertNormalOffset]);
				const vec3& normal2 = *reinterpret_cast<const vec3*>(&verts[idx2 + m_vertNormalOffset]);

				normal = normal0 * w + normal1 * u + normal2 * v;
			}
			else
				normal = cross(v1 - v0, v2 - v0); //calculate flat-shaded normal
		}
	}

	return hit;
}

//-------------------------------------------//

std::vector<std::shared_ptr<const Mesh>> Mesh::from_obj(std::string path)
{
	std::vector<std::shared_ptr<const Mesh>> result = {};

	//load meshes with qobj:
	//---------------
	uint32_t numMeshes;
	QOBJmesh* meshes;

	QOBJerror err = qobj_load_obj(path.c_str(), &numMeshes, &meshes);
	if(err != QOBJ_SUCCESS)
	{
		std::cout << "ERROR: failed to load qobj failed to load obj file \"" << path << "\" with error " << err << std::endl;
		return {};
	}

	//convert to Mesh object:
	//---------------
	for(uint32_t i = 0; i < numMeshes; i++)
	{
		std::unique_ptr<uint32_t[]> indices = std::unique_ptr<uint32_t[]>(new uint32_t[meshes[i].numIndices]);
		std::unique_ptr<float[]> verts = std::unique_ptr<float[]>(new float[meshes[i].numVertices * meshes[i].vertexStride]);

		//unfortuntaely we need to copy the data returned by qobj since we need unique_ptrs - TODO: fix this!!
		//(we cant use a custom deleter since we have 2 separate unique_ptrs)

		memcpy(indices.get(), meshes[i].indices, meshes[i].numIndices * sizeof(uint32_t));
		memcpy(verts.get(), meshes[i].vertices, meshes[i].numVertices * sizeof(float) * meshes[i].vertexStride);

		uint32_t attribs = 0;
		if(meshes[i].vertexAttribs & QOBJ_VERTEX_ATTRIB_POSITION)
			attribs |= VERTEX_ATTRIB_POSITION;
		if(meshes[i].vertexAttribs & QOBJ_VERTEX_ATTRIB_TEX_COORDS)
			attribs |= VERTEX_ATTRIB_UV;
		if(meshes[i].vertexAttribs & QOBJ_VERTEX_ATTRIB_NORMAL)
			attribs |= VERTEX_ATTRIB_NORMAL;

		result.push_back(std::make_shared<Mesh>(attribs, meshes[i].numIndices / 3, std::move(indices), std::move(verts), "", 
		                 meshes[i].vertexStride, meshes[i].vertexPosOffset, meshes[i].vertexTexCoordOffset, meshes[i].vertexNormalOffset));
	}

	//cleanup:
	//---------------
	qobj_free_obj(numMeshes, meshes);

	return result;
}

std::shared_ptr<const Mesh> Mesh::unit_sphere(uint32_t numSubdivisions, bool smoothNormals)
{
	std::shared_ptr<const Mesh> mesh;
	if(m_unitSpheres.find({numSubdivisions, smoothNormals}) != m_unitSpheres.end())
		mesh = m_unitSpheres[{numSubdivisions, smoothNormals}];
	else
	{
		mesh = gen_unit_sphere(numSubdivisions, smoothNormals);
		m_unitSpheres.insert({{numSubdivisions, smoothNormals}, mesh});
	}

	return mesh;
}


std::shared_ptr<const Mesh> Mesh::unit_cube()
{
	return m_unitCube;
}

std::shared_ptr<const Mesh> Mesh::unit_square()
{
	return m_unitSquare;
}

//-------------------------------------------//

bool Mesh::intersect_triangle(const Ray& ray, const vec3& v0, const vec3& v1, const vec3& v2, float& t, float& u, float& v) const
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
	return t > RURT_EPSILON;
}

void Mesh::setup_strides_offsets()
{
	//vertices must have a position, or the mesh isnt renderable:
	//---------------
	if((m_vertAttribs & VERTEX_ATTRIB_POSITION) == 0)
	{
		//TODO: proper error logging/handling
		std::cout << "ERROR: vertices MUST contain the POSITION attribute" << std::endl;
		m_valid = false;
		return;
	}

	//calculate stride:
	//---------------
	if(m_vertStride == UINT32_MAX)
	{
		m_vertStride = 0;

		if((m_vertAttribs & VERTEX_ATTRIB_POSITION) != 0)
			m_vertStride += 3;
		if((m_vertAttribs & VERTEX_ATTRIB_UV) != 0)
			m_vertStride += 2;
		if((m_vertAttribs & VERTEX_ATTRIB_NORMAL) != 0)
			m_vertStride += 3;
	}

	//calculate position offset (assume it comes first):
	//---------------
	if(m_vertPosOffset == UINT32_MAX)
		m_vertPosOffset = 0;

	//calculate uv offset (assume it comes directly after position):
	//---------------
	if(m_vertUvOffset == UINT32_MAX)
	{
		m_vertUvOffset = 0;

		if((m_vertAttribs & VERTEX_ATTRIB_POSITION) != 0)
			m_vertUvOffset += 3;
	}

	//calculate normal offset (assume it comes after position and uv):
	//---------------
	if(m_vertNormalOffset == UINT32_MAX)
	{
		m_vertNormalOffset = 0;

		if((m_vertAttribs & VERTEX_ATTRIB_POSITION) != 0)
			m_vertUvOffset += 3;
		if((m_vertAttribs & VERTEX_ATTRIB_UV) != 0)
			m_vertUvOffset += 2;
	}
}

//-------------------------------------------//

std::shared_ptr<const Mesh> Mesh::gen_unit_sphere(uint32_t numSubdivisions, bool smoothNormals)
{
	//create initial vertices and indices:
	//---------------
	const float x = 0.525731112119133606f;
	const float z = 0.850650808352039932f;
	const float n = 0.0f;
	
	std::vector<float> vertices= {
		-x, n, z,  
		 x, n, z, 
		-x, n,-z,  
		 x, n,-z,
		 n, z, x,  
		 n, z,-x,  
		 n,-z, x,  
		 n,-z,-x,
		 z, x, n, 
		-z, x, n,  
		 z,-x, n, 
		-z,-x, n
	};

	std::vector<uint32_t> indices = {
		 0,  4,  1,  
		 0,  9,  4,  
		 9,  5,  4,  
		 4,  5,  8,   
		 4,  8,  1,
		 8, 10,  1,  
		 8,  3, 10,  
		 5,  3,  8,  
		 5,  2,  3,   
		 2,  7,  3,
		 7, 10,  3,  
		 7,  6, 10,  
		 7, 11,  6, 
		11,  0,  6,  
		 0,  1,  6,
		 6,  1, 10,  
		 9,  0, 11,  
		 9, 11,  2,  
		 9,  2,  5,   
		 7,  2, 11
	};

	std::unordered_map<std::pair<uint32_t, uint32_t>, uint32_t, HashPair> vertexMap;

	//subdivide:
	//---------------
	for(uint32_t i = 0; i < numSubdivisions; i++)
		indices = icosphere_subdivide(vertices, indices, vertexMap);

	//copy to shared ptrs (no way to transfer ownership) and return mesh:
	//---------------
	std::unique_ptr<float[]> verticesUnique = std::unique_ptr<float[]>(new float[vertices.size()]);
	std::unique_ptr<uint32_t[]> indicesUnique = std::unique_ptr<uint32_t[]>(new uint32_t[indices.size()]);

	std::copy(vertices.begin(), vertices.end(), verticesUnique.get());
	std::copy(indices.begin(), indices.end(), indicesUnique.get());

	uint32_t attribs = VERTEX_ATTRIB_POSITION;
	if(smoothNormals)
		attribs |= VERTEX_ATTRIB_NORMAL;

	return std::make_shared<Mesh>(attribs, (uint32_t)indices.size() / 3, std::move(indicesUnique), 
	                              std::move(verticesUnique), "", 3, 0, UINT32_MAX, 0);
}

std::shared_ptr<const Mesh> Mesh::gen_unit_cube()
{
	float vertices[] = {
		-0.5f, -0.5f,  0.5f,
		 0.5f, -0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,
		-0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f, -0.5f,
		 0.5f,  0.5f, -0.5f,
		-0.5f,  0.5f, -0.5f
	};
	uint32_t numVertices = sizeof(vertices) / sizeof(float);
	std::unique_ptr<float[]> verticesUnique = std::unique_ptr<float[]>(new float[numVertices]);
	memcpy(verticesUnique.get(), vertices, sizeof(vertices));

	uint32_t indices[] = {
		0, 1, 2,
		0, 2, 3,
		5, 4, 7,
		5, 7, 6,
		4, 0, 3,
		4, 3, 7,
		1, 5, 6,
		1, 6, 2,
		3, 2, 6,
		3, 6, 7,
		4, 1, 0,
		4, 5, 1
	};
	uint32_t numIndices = sizeof(indices) / sizeof(uint32_t);
	std::unique_ptr<uint32_t[]> indicesUnique = std::unique_ptr<uint32_t[]>(new uint32_t[numIndices]);
	memcpy(indicesUnique.get(), indices, sizeof(indices));

	return std::make_shared<Mesh>(VERTEX_ATTRIB_POSITION, numIndices / 3, std::move(indicesUnique), std::move(verticesUnique), "", 3, 0);
}

std::shared_ptr<const Mesh> Mesh::gen_unit_square()
{
	float vertices[] = {
		-0.5f,  0.0f,  0.5f,
		 0.5f,  0.0f,  0.5f,
		 0.5f,  0.0f, -0.5f,
		-0.5f,  0.0f, -0.5f
	};
	uint32_t numVertices = sizeof(vertices) / sizeof(float);
	std::unique_ptr<float[]> verticesUnique = std::unique_ptr<float[]>(new float[numVertices]);
	memcpy(verticesUnique.get(), vertices, sizeof(vertices));

	uint32_t indices[] = {
		0, 1, 2,
		0, 2, 3
	};
	uint32_t numIndices = sizeof(indices) / sizeof(uint32_t);
	std::unique_ptr<uint32_t[]> indicesUnique = std::unique_ptr<uint32_t[]>(new uint32_t[numIndices]);
	memcpy(indicesUnique.get(), indices, sizeof(indices));

	return std::make_shared<Mesh>(VERTEX_ATTRIB_POSITION, numIndices / 3, std::move(indicesUnique), std::move(verticesUnique), "", 3, 0);
}

std::vector<uint32_t> Mesh::icosphere_subdivide(std::vector<float>& vertices, const std::vector<uint32_t>& indices, 
                                                std::unordered_map<std::pair<uint32_t, uint32_t>, uint32_t, HashPair>& vertexMap)
{
	std::vector<uint32_t> result;

	for(uint32_t i = 0; i < indices.size(); i += 3)
	{
		uint32_t mid[3];
		for(uint32_t j = 0; j < 3; j++)
		{
			uint32_t edge1 = indices[i + j];
			uint32_t edge2 = indices[i + ((j + 1) % 3)];
			if(edge1 > edge2)
			{
				uint32_t temp = edge1;
				edge1 = edge2;
				edge2 = temp;
			}

			std::pair<uint32_t, uint32_t> key = std::make_pair(edge1, edge2);
			auto idx = vertexMap.insert({key, (uint32_t)vertices.size() / 3});
			if(idx.second)
			{
				float x1 = vertices[edge1 * 3 + 0];
				float y1 = vertices[edge1 * 3 + 1];
				float z1 = vertices[edge1 * 3 + 2];

				float x2 = vertices[edge2 * 3 + 0];
				float y2 = vertices[edge2 * 3 + 1];
				float z2 = vertices[edge2 * 3 + 2];

				float newX = x1 + x2;
				float newY = y1 + y2;
				float newZ = z1 + z2;
				
				float len = sqrtf(newX * newX + newY * newY + newZ * newZ);
				newX /= len;
				newY /= len;
				newZ /= len;

				vertices.push_back(newX);
				vertices.push_back(newY);
				vertices.push_back(newZ);
			}

			mid[j] = idx.first->second;
		}

		result.push_back(indices[i + 0]);
		result.push_back(mid[0]);
		result.push_back(mid[2]);

		result.push_back(indices[i + 1]);
		result.push_back(mid[1]);
		result.push_back(mid[0]);

		result.push_back(indices[i + 2]);
		result.push_back(mid[2]);
		result.push_back(mid[1]);

		result.push_back(mid[0]);
		result.push_back(mid[1]);
		result.push_back(mid[2]);
	}

	return result;
}

}; //namespace rurt