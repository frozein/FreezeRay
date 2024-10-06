/* mesh.hpp
 *
 * contains the definition of the mesh class, which represents
 * a single 3D mesh composed of multiple polygons
 */

#ifndef RURT_MESH_H
#define RURT_MESH_H

#include <stdint.h>
#include <memory>
#include "ray.hpp"

#include "quickmath.hpp"
using namespace qm;

//-------------------------------------------//

namespace rurt
{

class Mesh
{
public:
	Mesh(uint32_t numFaces, std::shared_ptr<uint32_t[]> faceIndices, std::shared_ptr<uint32_t[]> vertIndices, std::shared_ptr<vec3[]> verts);
	~Mesh();

	bool intersect(const Ray& ray, float& t, float& u, float& v);

private:
	uint32_t m_numFaces;
	uint32_t m_numTris;
	std::shared_ptr<uint32_t[]> m_faceIndices;
	std::unique_ptr<uint32_t[]> m_vertIndices;
	std::shared_ptr<vec3[]> m_verts;

	bool intersect_triangle(const Ray& ray, const vec3& v0, const vec3& v1, const vec3& v2, float& t, float& u, float& v);
};

}; //namespace rurt

#endif //#ifndef RURT_MESH_H