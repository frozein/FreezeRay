/* fr_mesh.hpp
 *
 * contains the definition of the mesh class, which represents
 * a single 3D mesh composed of multiple polygons
 */

#ifndef FR_MESH_H
#define FR_MESH_H

#include <stdint.h>
#include <memory>
#include <vector>
#include <unordered_map>
#include "fr_ray.hpp"
#include "fr_raycast_info.hpp"

//-------------------------------------------//

namespace fr
{

//bitfield of possible attributes
enum VertexAttributes : uint32_t
{
	VERTEX_ATTRIB_POSITION = (1 << 0),
	VERTEX_ATTRIB_UV       = (1 << 1),
	VERTEX_ATTRIB_NORMAL   = (1 << 2)
};

class Mesh
{
public:
	Mesh(uint32_t vertexAttribs, uint32_t numFaces, std::unique_ptr<uint32_t[]> faceIndices, std::unique_ptr<uint32_t[]> vertIndices, 
	     std::unique_ptr<float[]> verts, std::string material = "", uint32_t vertStride = UINT32_MAX, uint32_t vertPosOffset = UINT32_MAX, 
		 uint32_t vertUvOffset = UINT32_MAX, uint32_t vertNormalOffset = UINT32_MAX);
	Mesh(uint32_t vertexAttribs, uint32_t numTris, std::unique_ptr<uint32_t[]> indices, std::unique_ptr<float[]> verts, 
	     std::string material = "", uint32_t vertStride = UINT32_MAX, uint32_t vertPosOffset = UINT32_MAX, 
		 uint32_t vertUvOffset = UINT32_MAX, uint32_t vertNormalOffset = UINT32_MAX);

	const std::string& get_material() const;
	void set_material(const std::string& material);

	uint32_t get_num_tris() const;
	void get_tri_indices(uint32_t triIdx, uint32_t& idx0, uint32_t& idx1, uint32_t& idx2) const;
	vec3 get_vert_pos_at(uint32_t idx) const;
	vec2 get_vert_uv_at(uint32_t idx) const;
	vec3 get_vert_normal_at(uint32_t idx) const;

	bool intersect(const Ray& ray, float& t, vec2& uv, vec3& normal, IntersectionInfo::Derivatives& derivs) const;

	//-------------------------------------------//

	static std::vector<std::shared_ptr<const Mesh>> from_obj(std::string path);
	static std::shared_ptr<const Mesh> unit_sphere(uint32_t numSubdivisions = 2, bool smoothNormals = true);
	static std::shared_ptr<const Mesh> unit_cube();
	static std::shared_ptr<const Mesh> unit_square();

private:
	bool m_valid = true; //whether or not this mesh is valid, can become invalid if given nonsense params

	std::string m_material;

	uint32_t m_vertAttribs;
	uint32_t m_vertStride;
	uint32_t m_vertPosOffset;
	uint32_t m_vertUvOffset;
	uint32_t m_vertNormalOffset;

	uint32_t m_numTris;
	std::unique_ptr<uint32_t[]> m_indices;
	std::unique_ptr<float[]> m_verts;

	bool intersect_triangle(const Ray& ray, const vec3& v0, const vec3& v1, const vec3& v2, float& t, float& u, float& v) const;
	void intersect_triangle_no_bounds_check(const Ray& ray, const vec3& v0, const vec3& v1, const vec3& v2, float& t, float& u, float& v) const;

	void setup_strides_offsets();

	//-------------------------------------------//

	struct HashPair 
	{
		template <class T1, class T2>
		size_t operator()(const std::pair<T1, T2>& p) const
		{
			auto hash1 = std::hash<T1>{}(p.first);
			auto hash2 = std::hash<T2>{}(p.second);

			if (hash1 != hash2)
				return hash1 ^ hash2;

			// If hash1 == hash2, their XOR is zero
			return hash1;
		}
	};

	static std::unordered_map<std::pair<uint32_t, bool>, std::shared_ptr<const Mesh>, HashPair> m_unitSpheres;
	static std::shared_ptr<const Mesh> m_unitCube;
	static std::shared_ptr<const Mesh> m_unitSquare;

	static std::shared_ptr<const Mesh> gen_unit_sphere(uint32_t numSubdivisions, bool smoothNormals);
	static std::shared_ptr<const Mesh> gen_unit_cube();
	static std::shared_ptr<const Mesh> gen_unit_square();

	static std::vector<uint32_t> icosphere_subdivide(std::vector<float>& vertices, const std::vector<uint32_t>& indices, 
	                                                 std::unordered_map<std::pair<uint32_t, uint32_t>, uint32_t, HashPair>& vertexMap);
};

}; //namespace fr

#endif //#ifndef FR_MESH_H