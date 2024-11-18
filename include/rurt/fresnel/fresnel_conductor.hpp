/* fresnel_conductor.hpp
 *
 * contains a definition for a conductor fresnel function
 */

#ifndef RURT_FRESNEL_CONDUCTOR_H
#define RURT_FRESNEL_CONDUCTOR_H

#include "fresnel.hpp"

//-------------------------------------------//

namespace rurt
{

class FresnelConductor : public Fresnel
{
public:
	FresnelConductor(float etaI, float etaT, float absorption);

	float evaluate(float cosThetaI) const override;

private:
	float m_etaI;
	float m_etaT;
	float m_absorption;
};

} //namespace rurt

#endif //#ifndef RURT_FRESNEL_CONDUCTOR_H