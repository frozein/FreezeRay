/* camera.hpp
 *
 * contains the defintion of the camera class, which represents a 
 * point of reference to render from, as well as perspecive information
 */

#ifndef RURT_CAMERA_H
#define RURT_CAMERA_H

#include "quickmath.hpp"
using namespace qm;

//-------------------------------------------//

namespace rurt
{

class Camera
{
public:
	Camera(const vec3& pos, const vec3& lookDir, const vec3& upDir, float fov, float aspectRatio);

	const mat4& view() const { return m_view; }
	const mat4& proj() const { return m_proj; }

private:
	mat4 m_view;
	mat4 m_proj;
};

}; //namespace rurt

#endif //#ifndef RURT_CAMERA_H