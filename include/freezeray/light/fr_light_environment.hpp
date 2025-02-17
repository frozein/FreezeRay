/* fr_light_environment.hpp
 *
 * contains a definition for an environment map light source
 */

#ifndef FR_LIGHT_ENVIRONMENT_H
#define FR_LIGHT_ENVIRONMENT_H

#include "../fr_light.hpp"
#include "../fr_distribution.hpp"

//-------------------------------------------//

namespace fr
{

class LightEnvironment : public Light
{
public:
	LightEnvironment(std::unique_ptr<const vec3[]> image, uint32_t width, uint32_t height, float worldRadius = 1.0f);
	LightEnvironment(const std::string& path, float worldRadius = 1.0f);

	vec3 sample_li(const IntersectionInfo& hitInfo, const vec3& u, vec3& wiWorld, VisibilityTestInfo& vis, float& pdf) const override;
	float pdf_li(const IntersectionInfo& hitInfo, const vec3& w) const override;
	vec3 power() const override;

	vec3 le(const IntersectionInfo& hitInfo, const vec3& w) const override;

private:
	uint32_t m_width;
	uint32_t m_height;
	std::unique_ptr<const vec3[]> m_image;

	float m_area;
	float m_luminance;
	vec3 m_power;
	float m_worldRadius;

	struct TexelCoordinate
	{
		uint32_t u;
		uint32_t v;
	};
	std::unique_ptr<DistributionDiscrete<TexelCoordinate>> m_texelDistribution;

	vec3 get_texel(uint32_t u, uint32_t v) const;
	vec3 bilinear(const vec2& uv) const;

	vec2 sample_texel_area(const vec3& u, TexelCoordinate& texel, float& pdf) const;
	void create_texel_distribution();
};

}; //namespace fr

#endif //#ifndef FR_LIGHT_ENVIRONMENT_H