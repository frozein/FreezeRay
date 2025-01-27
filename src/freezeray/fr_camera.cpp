#include "freezeray/fr_camera.hpp"

//-------------------------------------------//

namespace fr
{

Camera::Camera(const vec3& pos, const vec3& lookDir, const vec3& upDir, float fov, float aspectRatio)
{
	const float NEAR_PLANE = 0.1f; //since we are raytracing these values don't actually mean anything, but they need to be set regardless
	const float FAR_PLANE = 1000.0f;

	m_view = look(pos, lookDir, upDir);
	m_proj = perspective(fov, aspectRatio, NEAR_PLANE, FAR_PLANE);
}

}; //namespace fr