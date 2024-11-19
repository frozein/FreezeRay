/* btdf_specular_reflection.hpp
 *
 * contains a definition for a specular BTDF model
 */

#ifndef RURT_BTDF_SPECULAR_H
#define RURT_BTDF_SPECULAR_H

#include "../bxdf.hpp"
#include "../fresnel.hpp"

//-------------------------------------------//

namespace rurt
{

class BTDFSpecular : public BXDF
{
public:
	BTDFSpecular(const vec3& color, float etaI, float etaT, std::shared_ptr<const Fresnel> fresnel);

	vec3 f(const HitInfo& info, const vec3& wi, const vec3& wo) const override;
	vec3 sample_f(const HitInfo& info, vec3& wi, const vec3& wo, float& pdf) const override;
	float pdf(const HitInfo& info, const vec3& wi, const vec3& wo) const override;

	bool is_delta() const override { return true; }

private:
	vec3 m_color;
	float m_etaI;
	float m_etaT;
	std::shared_ptr<const Fresnel> m_fresnel;
};

};

#endif //#ifndef RURT_BTDF_SPECULAR_H