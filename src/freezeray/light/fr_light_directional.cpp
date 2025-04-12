#include "freezeray/light/fr_light_directional.hpp"
#include "freezeray/fr_globals.hpp"
#include "freezeray/fr_scene.hpp"

//-------------------------------------------//

namespace fr
{

LightDirectional::LightDirectional(const vec3& dir, const vec3& intensity) :
	Light(true, true), m_dir(normalize(dir)), m_intensity(intensity), m_worldRadius(1.0f)
{

}

vec3 LightDirectional::sample_li(const IntersectionInfo& hitInfo, const vec3& u, vec3& wiWorld, VisibilityTestInfo& vis, float& pdf) const
{
	wiWorld = m_dir;
	pdf = 1.0f;
	vis.startPos = hitInfo.pos;
	vis.endPos = hitInfo.pos + m_dir * (2.0f * m_worldRadius);

	return m_intensity;
}

float LightDirectional::pdf_li(const IntersectionInfo& hitInfo, const vec3& w) const
{
	//delta light
	return 0.0f;
}

vec3 LightDirectional::sample_le(const vec3& u1, const vec3& u2, Ray& ray, vec3& normal, float& pdfPos, float& pdfDir) const
{
    float r = std::sqrt(u1.x);
    float theta = FR_2_PI * u1.y;
    vec2 diskPos = vec2(r * std::cos(theta), r * std::sin(theta));

	vec3 v1, v2;
	get_orthogonal(m_dir, v1, v2);

	vec3 pos = m_worldRadius * (diskPos.x * v1 + diskPos.y * v2);

	ray = Ray(pos + m_worldRadius * m_dir, -1.0f * m_dir);
	normal = -1.0f * m_dir;
	pdfPos = 1.0f / (FR_PI * m_worldRadius * m_worldRadius);
	pdfDir = 1.0f;

	return m_intensity;
}

void LightDirectional::pdf_le(const Ray& ray, const vec3& normal, float& pdfPos, float& pdfDir) const
{
	pdfPos = 1.0f / (FR_PI * m_worldRadius * m_worldRadius);
	pdfDir = 0.0f;
}

vec3 LightDirectional::power() const
{
	return FR_PI * m_intensity * m_worldRadius * m_worldRadius;
}

void LightDirectional::preprocess(std::shared_ptr<const Scene> scene)
{
	m_worldRadius = scene->get_world_radius();
}

}; //namespace fr