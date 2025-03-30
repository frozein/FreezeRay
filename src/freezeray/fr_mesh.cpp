#include "freezeray/fr_mesh.hpp"

#define QOBJ_IMPLEMENTATION
#include "freezeray/quickobj.h"
#include "freezeray/fr_globals.hpp"
#include <algorithm>

//-------------------------------------------//

#define FR_MESH_KDTREE_MAX_TRIS_PER_NODE 16
#define FR_MESH_KDTREE_TRAVERSAL_COST 1
#define FR_MESH_KDTREE_ISECT_COST 2
#define FR_MESH_KDTREE_EMPTY_BONUS 0.5f

//-------------------------------------------//

namespace fr
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

	//setup strides and offsets, build kd tree:
	//---------------
	vert_attribs_setup();
	kdtree_build();
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
	vert_attribs_setup();
	kdtree_build();
}

const std::string& Mesh::get_material() const
{
	return m_material;
}

void Mesh::set_material(const std::string& material)
{
	m_material = material;
}

uint32_t Mesh::get_attribs() const
{
	return m_vertAttribs;
}

uint32_t Mesh::get_num_tris() const
{
	return m_numTris;
}

bound3 Mesh::get_bounds() const
{
	return m_kdTreeBounds;
}

void Mesh::get_tri_indices(uint32_t triIdx, uint32_t& idx0, uint32_t& idx1, uint32_t& idx2) const
{
	triIdx *= 3;

	idx0 = m_indices[triIdx + 0];
	idx1 = m_indices[triIdx + 1];
	idx2 = m_indices[triIdx + 2];
}

vec3 Mesh::get_vert_pos_at(uint32_t idx) const
{
	return *reinterpret_cast<const vec3*>(&m_verts.get()[idx * m_vertStride + m_vertPosOffset]);
}

vec2 Mesh::get_vert_uv_at(uint32_t idx) const
{
	if((m_vertAttribs & VERTEX_ATTRIB_UV) == 0)
		return vec2(0.0f);
	
	return *reinterpret_cast<const vec2*>(&m_verts.get()[idx * m_vertStride + m_vertUvOffset]);
}

vec3 Mesh::get_vert_normal_at(uint32_t idx) const
{
	if((m_vertAttribs & VERTEX_ATTRIB_NORMAL) == 0)
		return vec3(0.0f);
	
	return *reinterpret_cast<const vec3*>(&m_verts.get()[idx * m_vertStride + m_vertNormalOffset]);
}

bool Mesh::intersect(const Ray& ray, std::shared_ptr<const Texture<float>> alphaMask, float& tMin, vec2& uv, vec3& normal, IntersectionInfo::Derivatives& derivs) const
{
	//get ray info:
	//---------------
	vec3 rayDir = ray.direction();
	vec3 rayPos = ray.origin();
	vec3 invRayDir = 1.0f / rayDir;

	//compute intersection with bounding box:
	//---------------
	vec3 tMinKD3 = (m_kdTreeBounds.min - rayPos) * invRayDir;
	vec3 tMaxKD3 = (m_kdTreeBounds.max - rayPos) * invRayDir;

	vec3 t1 = min(tMinKD3, tMaxKD3);
	vec3 t2 = max(tMinKD3, tMaxKD3);

	float tMinKD;
	if(t1.x > t1.y)
	{
		if(t1.x > t1.z)
			tMinKD = t1.x;
		else
			tMinKD = t1.z;
	}
	else
	{
		if(t1.y > t1.z)
			tMinKD = t1.y;
		else
			tMinKD = t1.z;
	}

	float tMaxKD = std::min(std::min(t2.x, t2.y), t2.z);

	//skip if bb wasnt hit
	if(tMaxKD < tMinKD || tMaxKD <= 0.0f)
		return false;

	//declare attributes for hit triangle:
	//---------------
	float* verts = m_verts.get();
	
	bool hit = false;

	tMin = INFINITY;
	uint32_t minIdx0, minIdx1, minIdx2;
	vec3 minV0, minV1, minV2;
	float minB0, minB1;

	//traverse kd tree in order:
	//---------------
	struct KDnodeToVisit
	{
		uint32_t nodeIdx;
		float tMin;
		float tMax;
	};
	
	const uint32_t MAX_KD_TRAVERSAL_DEPTH = 64;
	KDnodeToVisit nodesToVisit[MAX_KD_TRAVERSAL_DEPTH];
	uint32_t toVisitPos = 0;

	uint32_t nodeIdx = 0;
	while(true)
	{
		//early exit if intersection was already found
		if(tMin < tMinKD)
			break;

		//get node, process interior or leaf
		const KDtreeNode* node = &m_kdTree[nodeIdx];
		if(!node->is_leaf())
		{
			uint32_t axis = node->get_split_axis();
			float tPlane = (node->get_split_pos() - rayPos[axis]) * invRayDir[axis];

			uint32_t firstChild;
			uint32_t secondChild;
			bool belowFirst = (rayPos[axis] < node->get_split_pos()) ||
				(rayPos[axis] == node->get_split_pos() && rayDir[axis] <= 0.0f);
			if(belowFirst)
			{
				firstChild = nodeIdx + 1;
				secondChild = node->get_above_child_idx();
			}
			else
			{
				firstChild = node->get_above_child_idx();
				secondChild = nodeIdx + 1;
			}

			if(tPlane > tMaxKD || tPlane <= 0.0f)
				nodeIdx = firstChild;
			else if(tPlane < tMinKD)
				nodeIdx = secondChild;
			else
			{
				nodesToVisit[toVisitPos].nodeIdx = secondChild;
				nodesToVisit[toVisitPos].tMin = tPlane;
				nodesToVisit[toVisitPos].tMax = tMaxKD;
				toVisitPos++;

				nodeIdx = firstChild;
				tMaxKD = tPlane;
			}
		}
		else
		{
			uint32_t numTris = node->get_num_tris();
			for(uint32_t i = 0; i < numTris; i++)
			{
				uint32_t triIdx = m_kdTreeTriIndices[node->get_tri_indices_offset() + i] * 3;
				uint32_t idx0 = m_indices[triIdx + 0] * m_vertStride;
				uint32_t idx1 = m_indices[triIdx + 1] * m_vertStride;
				uint32_t idx2 = m_indices[triIdx + 2] * m_vertStride;
			
				const vec3& v0 = *reinterpret_cast<const vec3*>(&verts[idx0 + m_vertPosOffset]);
				const vec3& v1 = *reinterpret_cast<const vec3*>(&verts[idx1 + m_vertPosOffset]);
				const vec3& v2 = *reinterpret_cast<const vec3*>(&verts[idx2 + m_vertPosOffset]);
		
				float t;
				float b0, b1; //barycentric coordinates
				if(intersect_triangle(ray, v0, v1, v2, t, b0, b1) && t < tMin &&
				   test_alpha_mask(alphaMask, idx0, idx1, idx2, b0, b1))
				{
					hit |= true;
		
					tMin = t;
					minIdx0 = idx0;
					minIdx1 = idx1;
					minIdx2 = idx2;
					minV0 = v0;
					minV1 = v1;
					minV2 = v2;
					minB0 = b0;
					minB1 = b1;
				}
			}

			if(toVisitPos > 0)
			{
				toVisitPos--;
				nodeIdx = nodesToVisit[toVisitPos].nodeIdx;
				tMinKD = nodesToVisit[toVisitPos].tMin;
				tMaxKD = nodesToVisit[toVisitPos].tMax;
			}
			else
				break;
		}
	}

	//return early if not hit:
	//---------------
	if(!hit)
		return false;

	//get attributes:
	//---------------
	float b2 = 1.0f - minB0 - minB1;

	const vec2 *uv0, *uv1, *uv2;
	if((m_vertAttribs & VERTEX_ATTRIB_UV) != 0)
	{
		uv0 = reinterpret_cast<const vec2*>(&verts[minIdx0 + m_vertUvOffset]);
		uv1 = reinterpret_cast<const vec2*>(&verts[minIdx1 + m_vertUvOffset]);
		uv2 = reinterpret_cast<const vec2*>(&verts[minIdx2 + m_vertUvOffset]);

		uv = *uv0 * b2 + *uv1 * minB0 + *uv2 * minB1;
	}
	else
		uv = vec2(0.0f);

	vec3 geomNormal = cross(minV1 - minV0, minV2 - minV0); //geometric normal
	if((m_vertAttribs & VERTEX_ATTRIB_NORMAL) != 0)
	{
		const vec3& normal0 = *reinterpret_cast<const vec3*>(&verts[minIdx0 + m_vertNormalOffset]);
		const vec3& normal1 = *reinterpret_cast<const vec3*>(&verts[minIdx1 + m_vertNormalOffset]);
		const vec3& normal2 = *reinterpret_cast<const vec3*>(&verts[minIdx2 + m_vertNormalOffset]);

		//ensure that shaded normal is in "same hemisphere" as geom normal to avoid shading errors
		vec3 shadingNormal = normal0 * b2 + normal1 * minB0 + normal2 * minB1;
		if(dot(rayDir, geomNormal) * dot(rayDir, shadingNormal) <= 0.0f)
			normal = geomNormal;
		else
			normal = shadingNormal;
	}
	else
		normal = geomNormal;

	//calculate derivatives:
	//---------------
	if(ray.has_differentials())
	{
		vec3 p = ray.at(tMin);

		float tx;
		float b0x, b1x;
		intersect_triangle_no_bounds_check(ray.differential_x(), minV0, minV1, minV2, tx, b0x, b1x);
		vec3 px = ray.differential_x().at(tx);

		float ty;
		float b0y, b1y;
		intersect_triangle_no_bounds_check(ray.differential_y(), minV0, minV1, minV2, ty, b0y, b1y);
		vec3 py = ray.differential_y().at(ty);

		derivs.dpdx = px - p;
		derivs.dpdy = py - p;

		if((m_vertAttribs & VERTEX_ATTRIB_UV) != 0)
		{
			float b2x = 1.0f - b0x - b1x;
			derivs.duvdx = (*uv0 * b2x + *uv1 * b0x + *uv2 * b1x) - uv;

			float b2y = 1.0f - b0y - b1y;
			derivs.duvdy = (*uv0 * b2y + *uv1 * b0y + *uv2 * b1y) - uv;
		}
		else
		{
			derivs.duvdx = vec2(0.0f);
			derivs.duvdy = vec2(0.0f);
		}
	}
	else
		derivs = {0};

	return true;
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

		result.push_back(std::make_shared<Mesh>(attribs, meshes[i].numIndices / 3, std::move(indices), std::move(verts), meshes[i].material, 
						 meshes[i].vertexStride, meshes[i].vertexPosOffset, meshes[i].vertexTexCoordOffset, meshes[i].vertexNormalOffset));
	}

	//cleanup + return:
	//---------------
	qobj_free_obj(numMeshes, meshes);

	return result;
}

std::shared_ptr<const Mesh> Mesh::from_unit_sphere(uint32_t numSubdivisions, bool smoothNormals)
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


std::shared_ptr<const Mesh> Mesh::from_unit_cube()
{
	return m_unitCube;
}

std::shared_ptr<const Mesh> Mesh::from_unit_square()
{
	return m_unitSquare;
}

//-------------------------------------------//

bool Mesh::intersect_triangle(const Ray& ray, const vec3& v0, const vec3& v1, const vec3& v2, float& t, float& u, float& v)
{
	vec3 v0v1 = v1 - v0;
	vec3 v0v2 = v2 - v0;
	vec3 pvec = cross(ray.direction(), v0v2);
	float det = dot(v0v1, pvec);
	if(abs(det) < std::numeric_limits<float>::epsilon())
		return false;

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
	return t > FR_EPSILON;
}

bool Mesh::test_alpha_mask(std::shared_ptr<const Texture<float>> alphaMask, uint32_t idx0, uint32_t idx1, uint32_t idx2, float b0, float b1) const
{
	if(alphaMask == nullptr)
		return true;

	if((m_vertAttribs & VERTEX_ATTRIB_UV) == 0)
		return true;

	//get uv:
	//---------------
	float* verts = m_verts.get();
	const vec2* uv0 = reinterpret_cast<const vec2*>(&verts[idx0 + m_vertUvOffset]);
	const vec2* uv1 = reinterpret_cast<const vec2*>(&verts[idx1 + m_vertUvOffset]);
	const vec2* uv2 = reinterpret_cast<const vec2*>(&verts[idx2 + m_vertUvOffset]);
	
	float b2 = 1.0f - b0 - b1;
	vec2 uv = *uv0 * b2 + *uv1 * b0 + *uv2 * b1;

	//evaluate texture (just want the texel, no mipmapping):
	//---------------
	IntersectionInfo evalInfo;
	evalInfo.uv = uv;
	evalInfo.derivatives.dpdx = 0.0f;
	evalInfo.derivatives.dpdy = 0.0f;
	evalInfo.derivatives.duvdx = 0.0f;
	evalInfo.derivatives.duvdy = 0.0f;

	return alphaMask->evaluate(evalInfo) > 0.0f;
}

void Mesh::intersect_triangle_no_bounds_check(const Ray& ray, const vec3& v0, const vec3& v1, const vec3& v2, float& t, float& u, float& v)
{
	vec3 v0v1 = v1 - v0;
	vec3 v0v2 = v2 - v0;
	vec3 pvec = cross(ray.direction(), v0v2);
	float det = dot(v0v1, pvec);

	float invDet = 1.0f / det;

	vec3 tvec = ray.origin() - v0;
	u = dot(tvec, pvec) * invDet;

	vec3 qvec = cross(tvec, v0v1);
	v = dot(ray.direction(), qvec) * invDet;

	t = dot(v0v2, qvec) * invDet;
}

void Mesh::vert_attribs_setup()
{
	//vertices must have a position, or the mesh isnt renderable:
	//---------------
	if((m_vertAttribs & VERTEX_ATTRIB_POSITION) == 0)
		throw std::runtime_error("vertices MUST contain the POSITION attribute" );

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
			m_vertNormalOffset += 3;
		if((m_vertAttribs & VERTEX_ATTRIB_UV) != 0)
			m_vertNormalOffset += 2;
	}
}

//-------------------------------------------//

void Mesh::KDtreeNode::init_interior(uint32_t axis, uint32_t _aboveChildIdx, float splitPos)
{
	flags = axis;
	split = splitPos;
	aboveChildIdx |= (_aboveChildIdx << 2);
}

void Mesh::KDtreeNode::init_leaf(uint32_t _numTris, uint32_t* tris, std::vector<uint32_t>& triIndices)
{
	flags = 3;
	numTris |= (_numTris << 2);

	triIndicesOffset = (uint32_t)triIndices.size();
	for(uint32_t i = 0; i < _numTris; i++)
		triIndices.push_back(tris[i]);
}

void Mesh::kdtree_build()
{
	//compute heuristic for maximum tree depth:
	//---------------
	uint32_t maxDepth = (uint32_t)std::roundf(8.0f + 1.3f * std::log2f((float)m_numTris));

	//compute bounds for each triangle:
	//---------------
	std::vector<bound3> triBounds(m_numTris);
	bound3 bounds;

	for(uint32_t i = 0; i < m_numTris; i++)
	{
		uint32_t triIdx = i * 3;
		uint32_t idx0 = m_indices[triIdx + 0] * m_vertStride;
		uint32_t idx1 = m_indices[triIdx + 1] * m_vertStride;
		uint32_t idx2 = m_indices[triIdx + 2] * m_vertStride;

		const vec3& v0 = *reinterpret_cast<const vec3*>(&m_verts.get()[idx0 + m_vertPosOffset]);
		const vec3& v1 = *reinterpret_cast<const vec3*>(&m_verts.get()[idx1 + m_vertPosOffset]);
		const vec3& v2 = *reinterpret_cast<const vec3*>(&m_verts.get()[idx2 + m_vertPosOffset]);

		vec3 triMin = min(min(v0, v1), v2);
		vec3 triMax = max(max(v0, v1), v2);

		triBounds[i] = { triMin, triMax };
		bounds.min = min(bounds.min, triMin);
		bounds.max = max(bounds.max, triMax);
	}

	m_kdTreeBounds = bounds;

	//allocate scratch mem:
	//---------------
	std::unique_ptr<KDtreeBoundEdge[]> scratchBufBoundEdges[3];
	for(uint32_t i = 0; i < 3; i++)
		scratchBufBoundEdges[i] = std::unique_ptr<KDtreeBoundEdge[]>(new KDtreeBoundEdge[m_numTris * 2]);

	std::unique_ptr<uint32_t[]> scratchBufTrisBelow(new uint32_t[m_numTris]);
	std::unique_ptr<uint32_t[]> scratchBufTrisAbove(new uint32_t[(maxDepth + 1) * m_numTris]);

	//begin recursive build:
	//---------------
	for(uint32_t i = 0; i < m_numTris; i++)
		scratchBufTrisBelow[i] = i;

	uint32_t treeSize = 0;
	uint32_t treeNextFree = 0;

	kdtree_build_recursive(
		0, &treeSize, &treeNextFree, bounds, triBounds, m_numTris, scratchBufTrisBelow.get(), maxDepth,
		scratchBufBoundEdges, scratchBufTrisBelow.get(), scratchBufTrisAbove.get()
	);
}

void Mesh::kdtree_build_recursive(uint32_t idx, uint32_t* treeSize, uint32_t* treeNextFree,
                                   const bound3& bounds, const std::vector<bound3>& triBounds, 
                                   uint32_t numTris, uint32_t* tris, uint32_t depth, 
                                   const std::unique_ptr<KDtreeBoundEdge[]> boundEdges[3],
                                   uint32_t* trisBelow, uint32_t* trisAbove)
{
	//resize tree if needed:
	//---------------
	if(*treeNextFree >= *treeSize)
	{
		uint32_t newTreeSize = std::max(2 * (*treeSize), 256u);
		std::unique_ptr<KDtreeNode[]> newTree = std::unique_ptr<KDtreeNode[]>(new KDtreeNode[newTreeSize]);

		if(*treeSize > 0) 
			std::memcpy(newTree.get(), m_kdTree.get(), (*treeSize) * sizeof(KDtreeNode));

		m_kdTree = std::move(newTree);
		*treeSize = newTreeSize;
	}

	(*treeNextFree)++;

	//stop recursion if at max depth or max tris:
	//---------------
	if(numTris <= FR_MESH_KDTREE_MAX_TRIS_PER_NODE || depth == 0)
	{
		m_kdTree[idx].init_leaf(numTris, tris, m_kdTreeTriIndices);
		return;
	}

	//find split axis + position:
	//---------------
	int32_t bestAxis = -1;
	int32_t bestOffset = -1;
	float minCost = INFINITY;

	vec3 d = bounds.max - bounds.min;
	float totalSA = 2.0f * (d.x * d.y + d.x * d.z + d.y * d.z);
	float invTotalSA = 1.0f / totalSA;

	uint32_t axes[3];
	if(d.x > d.y)
	{
		if(d.x > d.z)
		{
			axes[0] = 0;
			axes[1] = (d.y > d.z) ? 1 : 2;
			axes[2] = (d.y > d.z) ? 2 : 1;
		}
		else
		{
			axes[0] = 2;
			axes[1] = (d.x > d.y) ? 0 : 1;
			axes[2] = (d.x > d.y) ? 1 : 0;
		}
	}
	else
	{
		if(d.y > d.z)
		{
			axes[0] = 1;
			axes[1] = (d.x > d.z) ? 0 : 2;
			axes[2] = (d.x > d.z) ? 2 : 0;
		}
		else
		{
			axes[0] = 2;
			axes[1] = (d.x > d.y) ? 0 : 1;
			axes[2] = (d.x > d.y) ? 1 : 0;
		}
	}

	for(uint32_t i = 0; i < 3; i++)
	{
		uint32_t axis = axes[i];

		//init bound edges
		for(uint32_t j = 0; j < numTris; j++)
		{
			uint32_t tri = tris[j];
			const bound3& b = triBounds[tri];

			boundEdges[axis][2 * j    ] = {tri, b.min[axis], true};
			boundEdges[axis][2 * j + 1] = {tri, b.max[axis], false};
		}

		std::sort(&boundEdges[axis][0], &boundEdges[axis][2 * numTris],
			[](const KDtreeBoundEdge &e0, const KDtreeBoundEdge &e1) -> bool {
				if(e0.pos == e1.pos)
				{
					if(e0.start == e1.start)
						return e0.tri < e1.tri;
					else
						return (int)e0.start < (int)e1.start;
				}
				else 
					return e0.pos < e1.pos; 
			}
		);

		//compute cost at each potential split, find the best
		uint32_t numTrisBelow = 0;
		uint32_t numTrisAbove = numTris;

		for(uint32_t j = 0; j < 2 * numTris; j++)
		{
			if(!boundEdges[axis][j].start)
				numTrisAbove--;

			float edgePos = boundEdges[axis][j].pos;
			if(edgePos > bounds.min[axis] && edgePos < bounds.max[axis])
			{
				//get sa
				uint32_t otherAxis1 = (axis + 1) % 3;
				uint32_t otherAxis2 = (axis + 2) % 3;
				float belowSA = 2 * (
					d[otherAxis1] * d[otherAxis2] +
				    (edgePos - bounds.min[axis]) * (d[otherAxis1] + d[otherAxis2])
				);
				float aboveSA = 2 * (
					d[otherAxis1] * d[otherAxis2] +
					(bounds.max[axis] - edgePos) * (d[otherAxis1] + d[otherAxis2])
				);

				//compute cost
				float belowP = belowSA * invTotalSA;
				float aboveP = aboveSA * invTotalSA;
				float emptyBonus = (numTrisAbove == 0 || numTrisBelow == 0) ? FR_MESH_KDTREE_EMPTY_BONUS : 0.0f;

				float cost = FR_MESH_KDTREE_TRAVERSAL_COST + 
					FR_MESH_KDTREE_ISECT_COST * (1.0f - emptyBonus) * (belowP * numTrisBelow + aboveP * numTrisAbove);
				if(cost < minCost)
				{
					minCost = cost;
					bestAxis = axis;
					bestOffset = j;
				}
			}

			if(boundEdges[axis][j].start)
				numTrisBelow++;
		}

		//exit if any good split was found
		if(bestAxis >= 0)
			break;
	}

	//determine if we should split:
	//---------------
	float leafCost = FR_MESH_KDTREE_ISECT_COST * (float)numTris;

	//TODO: better heuristic on whether to split/not split?
	if(bestAxis == -1 || minCost > leafCost)
	{
		m_kdTree[idx].init_leaf(numTris, tris, m_kdTreeTriIndices);
		return;
	}
	
	//get triangles above and below split:
	//---------------
	uint32_t numTrisBelow = 0;
	for(uint32_t i = 0; i < (uint32_t)bestOffset; i++)
		if(boundEdges[bestAxis][i].start)
			trisBelow[numTrisBelow++] = boundEdges[bestAxis][i].tri;

	uint32_t numTrisAbove = 0;
	for(uint32_t i = bestOffset + 1; i < 2 * numTris; i++)
		if(!boundEdges[bestAxis][i].start)
			trisAbove[numTrisAbove++] = boundEdges[bestAxis][i].tri;

	//recursively build:
	//---------------
	bound3 belowBounds = bounds;
	bound3 aboveBounds = bounds;
			
	float splitPos = boundEdges[bestAxis][bestOffset].pos;
	belowBounds.max[bestAxis] = aboveBounds.min[bestAxis] = splitPos;

	uint32_t belowIdx = idx + 1;
	kdtree_build_recursive(
		belowIdx, treeSize, treeNextFree, belowBounds, triBounds, 
		numTrisBelow, trisBelow, depth - 1, 
		boundEdges, trisBelow, trisAbove + numTris
	);

	uint32_t aboveIdx = *treeNextFree;
	kdtree_build_recursive(
		aboveIdx, treeSize, treeNextFree, aboveBounds, triBounds, 
		numTrisAbove, trisAbove, depth - 1, 
		boundEdges, trisBelow, trisAbove + numTris
	);

	//init node:
	//---------------
	m_kdTree[idx].init_interior(bestAxis, aboveIdx, splitPos);
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
		 0,  1,  4,
		 0,  4,  9,  
		 9,  4,  5,  
		 4,  8,  5,   
		 4,  1,  8,
		 8,  1, 10,  
		 8, 10,  3,  
		 5,  8,  3,  
		 5,  3,  2,   
		 2,  3,  7,
		 7,  3, 10,  
		 7, 10,  6,  
		 7,  6, 11, 
		11,  6,  0,  
		 0,  6,  1,
		 6, 10,  1,  
		 9, 11,  0,  
		 9,  2, 11,  
		 9,  5,  2,   
		 7, 11,  2
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
		-0.5f,  0.0f, -0.5f, 0.0f, 0.0f,
		-0.5f,  0.0f,  0.5f, 0.0f, 1.0f,
		 0.5f,  0.0f,  0.5f, 1.0f, 1.0f,
		 0.5f,  0.0f, -0.5f, 1.0f, 0.0f
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

	return std::make_shared<Mesh>(VERTEX_ATTRIB_POSITION | VERTEX_ATTRIB_UV, numIndices / 3, std::move(indicesUnique), std::move(verticesUnique), "", 5, 0, 3);
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

}; //namespace fr