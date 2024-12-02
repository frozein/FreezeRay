/* fresnel_conductor.hpp
 *
 * contains a definition for a conductor fresnel function
 */

#ifndef RURT_FRESNEL_CONDUCTOR_H
#define RURT_FRESNEL_CONDUCTOR_H

#include "../fresnel.hpp"

//-------------------------------------------//

namespace rurt
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

} //namespace rurt

#endif //#ifndef RURT_FRESNEL_CONDUCTOR_H