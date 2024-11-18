#include "rurt/fresnel/fresnel_dielectric.hpp"
#include "rurt/globals.hpp"

//-------------------------------------------//

namespace rurt
{

FresnelDielectric::FresnelDielectric(float etaI, float etaT) :
	m_etaI(etaI), m_etaT(etaT)
{

}

float FresnelDielectric::evaluate(float cosThetaI) const
{
	float etaI;
	float etaT;
	if(cosThetaI > 0.0f) //not entering, need to swap
	{
		etaI = m_etaI;
		etaT = m_etaT;
	}
	else
	{
		etaI = m_etaT;
		etaT = m_etaI;
	}
	
	float sinThetaI = std::sqrtf(1.0f - cosThetaI * cosThetaI);
	float sinThetaT = (etaI / etaT) * sinThetaI;
	if(sinThetaT >= 1.0f)
		return 1.0f;

	float cosThetaT = std::sqrtf(1.0f - sinThetaT * sinThetaT);

	float rpar  = ((etaT * cosThetaI) - (etaI * cosThetaT)) / ((etaT * cosThetaI) + (etaI * cosThetaT));
	float rperp = ((etaI * cosThetaI) - (etaT * cosThetaT)) / ((etaI * cosThetaI) + (etaT * cosThetaT));
	return (rpar * rpar + rperp * rperp) / 2.0f;
}

}; //namespace rurt