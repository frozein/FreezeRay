/* fr_fresnel_conductor.hpp
 *
 * contains a definition for a conductor fresnel function
 */

#ifndef FR_FRESNEL_CONDUCTOR_H
#define FR_FRESNEL_CONDUCTOR_H

#include "../fr_fresnel.hpp"

//-------------------------------------------//

namespace fr
{

class FresnelConductor : public Fresnel
{
public:
	FresnelConductor(const vec3& etaI, const vec3& etaT, const vec3& absorption);

	vec3 evaluate(float cosThetaI) const override;

private:
	vec3 m_etaI;
	vec3 m_etaT;
	vec3 m_absorption;
};

} //namespace fr

#endif //#ifndef FR_FRESNEL_CONDUCTOR_H