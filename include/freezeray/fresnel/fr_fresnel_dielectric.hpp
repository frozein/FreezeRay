/* fr_fresnel_dielectric.hpp
 *
 * contains a definition for a dielecctric fresnel function
 */

#ifndef FR_FRESNEL_DIELECTRIC
#define FR_FRESNEL_DIELECTRIC

#include "../fr_fresnel.hpp"

//-------------------------------------------//

namespace fr
{

class FresnelDielectric : public Fresnel
{
public:
	FresnelDielectric(float etaI, float etaT);

	vec3 evaluate(float cosThetaI) const override;

private:
	float m_etaI;
	float m_etaT;
};

} //namespace fr

#endif //#ifndef FR_FRESNEL_DIELECTRIC