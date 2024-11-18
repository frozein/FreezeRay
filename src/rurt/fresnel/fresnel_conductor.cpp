#include "rurt/fresnel/fresnel_conductor.hpp"
#include "rurt/globals.hpp"

//-------------------------------------------//

namespace rurt
{

FresnelConductor::FresnelConductor(float etaI, float etaT, float absorption) :
	m_etaI(etaI), m_etaT(etaT), m_absorption(absorption)
{

}

float FresnelConductor::evaluate(float cosThetaI) const
{
	float eta = m_etaT / m_etaI;
	float etaK = m_absorption / m_etaI;

	float cosThetaI2 = cosThetaI * cosThetaI;
	float sinThetaI2 = 1.0f - cosThetaI2;
	float eta2 = eta * eta;
	float etaK2 = etaK * etaK;

    float t0 = eta2 - etaK2 - sinThetaI2;
    float a2b2 = std::sqrtf(t0 * t0 + 4.0f * eta2 * etaK2);
    float t1 = a2b2 + cosThetaI2;
    float a = std::sqrtf(0.5f * (a2b2 + t0));
    float t2 = 2.0f * cosThetaI * a;
    float rs = (t1 - t2) / (t1 + t2);

    float t3 = cosThetaI2 * a2b2 + sinThetaI2 * sinThetaI2;
    float t4 = t2 * sinThetaI2;
    float rp = rs * (t3 - t4) / (t3 + t4);

    return (rp + rs) / 2.0f;
}

}; //namespace rurt