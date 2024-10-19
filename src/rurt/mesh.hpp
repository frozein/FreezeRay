/* mesh.hpp
 *
 * contains the definition of the mesh class, which represents
 * a single 3D mesh composed of multiple polygons
 */

#ifndef RURT_MESH_H
#define RURT_MESH_H

#include <stdint.h>
#include <memory>
#include <vector>
#include "ray.hpp"

#include "quickmath.hpp"
using namespace qm;

//-------------------------------------------//

namespace rurt
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
	Mesh(uint32_t vertexAttribs, uint32_t numFaces, std::shared_ptr<uint32_t[]> faceIndices, std::shared_ptr<uint32_t[]> vertIndices, 
	     std::shared_ptr<float[]> verts, std::string material = "", uint32_t vertStride = UINT32_MAX, uint32_t vertPosOffset = UINT32_MAX, 
		 uint32_t vertUvOffset = UINT32_MAX, uint32_t vertNormalOffset = UINT32_MAX);
	Mesh(uint32_t vertexAttribs, uint32_t numTris, std::shared_ptr<uint32_t[]> indices, std::shared_ptr<float[]> verts, 
	     std::string material = "", uint32_t vertStride = UINT32_MAX, uint32_t vertPosOffset = UINT32_MAX, 
		 uint32_t vertUvOffset = UINT32_MAX, uint32_t vertNormalOffset = UINT32_MAX);

	std::string get_material();

	bool intersect(const Ray& ray, float& t, vec2& uv, vec3& normal);

	static std::vector<std::shared_ptr<Mesh>> from_obj(std::string path);

private:
	bool m_valid = true; //whether or not this mesh is valid, can become invalid if given nonsense params

	std::string m_material;

	uint32_t m_vertAttribs;
	uint32_t m_vertStride;
	uint32_t m_vertPosOffset;
	uint32_t m_vertUvOffset;
	uint32_t m_vertNormalOffset;

	uint32_t m_numTris;
	std::shared_ptr<uint32_t[]> m_indices;
	std::shared_ptr<float[]> m_verts;

	bool intersect_triangle(const Ray& ray, const vec3& v0, const vec3& v1, const vec3& v2, float& t, float& u, float& v);

	void setup_strides_offsets();
};

}; //namespace rurt

#endif //#ifndef RURT_MESH_H