#include "rurt/fresnel/fresnel_constant.hpp"
#include "rurt/globals.hpp"

//-------------------------------------------//

namespace rurt
{

FresnelConstant::FresnelConstant(float value) :
	m_value(value)
{

}

float FresnelConstant::evaluate(float cosnThetaI) const
{
	return m_value;
}

}; //namespace rurt