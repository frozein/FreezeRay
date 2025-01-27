#include "freezeray/fresnel/fr_fresnel_dielectric.hpp"
#include "freezeray/fr_globals.hpp"

//-------------------------------------------//

namespace fr
{

FresnelDielectric::FresnelDielectric(float etaI, float etaT) :
	m_etaI(etaI), m_etaT(etaT)
{

}

vec3 FresnelDielectric::evaluate(float cosThetaI) const
{
	float etaI;
	float etaT;
	if(cosThetaI > 0.0f)
	{
		etaI = m_etaI;
		etaT = m_etaT;
	}
	else //not entering, need to swap
	{
		etaI = m_etaT;
		etaT = m_etaI;
		cosThetaI = -cosThetaI;
	}
	
	float sinThetaI = std::sqrtf(1.0f - cosThetaI * cosThetaI);
	float sinThetaT = (etaI / etaT) * sinThetaI;
	if(sinThetaT >= 1.0f)
		return 1.0f;

	float cosThetaT = std::sqrtf(1.0f - sinThetaT * sinThetaT);

	float rpar  = ((etaT * cosThetaI) - (etaI * cosThetaT)) / ((etaT * cosThetaI) + (etaI * cosThetaT));
	float rperp = ((etaI * cosThetaI) - (etaT * cosThetaT)) / ((etaI * cosThetaI) + (etaT * cosThetaT));
	return vec3((rpar * rpar + rperp * rperp) / 2.0f);
}

}; //namespace fr