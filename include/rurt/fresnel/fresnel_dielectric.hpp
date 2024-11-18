/* fresnel_dielectric.hpp
 *
 * contains a definition for a dielecctric fresnel function
 */

#ifndef RURT_FRESNEL_DIELECTRIC
#define RURT_FRESNEL_DIELECTRIC

#include "fresnel.hpp"

//-------------------------------------------//

namespace rurt
{

class FresnelDielectric : public Fresnel
{
public:
	FresnelDielectric(float etaI, float etaT);

	float evaluate(float cosThetaI) const override;

private:
	float m_etaI;
	float m_etaT;
};

} //namespace rurt

#endif //#ifndef RURT_FRESNEL_DIELECTRIC