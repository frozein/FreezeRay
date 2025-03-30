#include "freezeray/light/fr_light_point.hpp"
#include "freezeray/fr_globals.hpp"

//-------------------------------------------//

namespace fr
{

LightPoint::LightPoint(const vec3& pos, const vec3& intensity) :
	Light(true, false), m_pos(pos), m_intensity(intensity)
{

}

vec3 LightPoint::sample_li(const IntersectionInfo& hitInfo, const vec3& u, vec3& wiWorld, VisibilityTestInfo& vis, float& pdf) const
{
	vec3 toLight = m_pos - hitInfo.worldPos;

	wiWorld = normalize(toLight);
	pdf = 1.0f;
	vis.infinite = false;
	vis.endPos = m_pos;

	return m_intensity / dot(toLight, toLight);
}

float LightPoint::pdf_li(const IntersectionInfo& hitInfo, const vec3& w) const
{
	//delta light
	return 1.0f;
}

vec3 LightPoint::sample_le(const vec3& u1, const vec3& u2, Ray& ray, vec3& normal, float& pdfPos, float& pdfDir) const
{
    float z = 1.0f - 2.0f * u1.x;
    float r = std::sqrt(std::max(0.0f, 1.0f - z * z));
    float theta = 2 * FR_PI * u1.y;
    vec3 dir(r * std::cos(theta), z, r * std::sin(theta));

	ray = Ray(m_pos, dir);
	normal = ray.direction();
	pdfPos = 1.0f;
	pdfDir = 1.0f / (4.0f * FR_PI);

	return m_intensity;
}

void LightPoint::pdf_le(const Ray& ray, const vec3& normal, float& pdfPos, float& pdfDir) const
{
	pdfPos = 0.0f;
	pdfDir = 1.0f / (4.0f * FR_PI);
}

vec3 LightPoint::power() const
{
	return 4.0f * FR_PI * m_intensity;
}

}; //namespace fr