#include "rurt/fresnel/fresnel_conductor.hpp"
#include "rurt/globals.hpp"

//-------------------------------------------//

namespace rurt
{

FresnelConductor::FresnelConductor(const vec3& etaI, const vec3& etaT, const vec3& absorption) :
	m_etaI(etaI), m_etaT(etaT), m_absorption(absorption)
{

}

vec3 FresnelConductor::evaluate(float cosThetaI) const
{
	vec3 eta = m_etaT / m_etaI;
	vec3 etaK = m_absorption / m_etaI;

	vec3 cosThetaI2 = cosThetaI * cosThetaI;
	vec3 sinThetaI2 = 1.0f - cosThetaI2;
	vec3 eta2 = eta * eta;
	vec3 etaK2 = etaK * etaK;

    vec3 t0 = eta2 - etaK2 - sinThetaI2;
    vec3 a2b2 = sqrt(t0 * t0 + 4.0f * eta2 * etaK2);
    vec3 t1 = a2b2 + cosThetaI2;
    vec3 a = sqrt(0.5f * (a2b2 + t0));
    vec3 t2 = 2.0f * cosThetaI * a;
    vec3 rs = (t1 - t2) / (t1 + t2);

    vec3 t3 = cosThetaI2 * a2b2 + sinThetaI2 * sinThetaI2;
    vec3 t4 = t2 * sinThetaI2;
    vec3 rp = rs * (t3 - t4) / (t3 + t4);

    return (rp + rs) / 2.0f;
}

}; //namespace rurt