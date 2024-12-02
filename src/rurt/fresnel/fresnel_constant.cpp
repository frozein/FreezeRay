#include "rurt/fresnel/fresnel_constant.hpp"
#include "rurt/globals.hpp"

//-------------------------------------------//

namespace rurt
{

FresnelConstant::FresnelConstant(const vec3& value) :
	m_value(value)
{

}

vec3 FresnelConstant::evaluate(float cosnThetaI) const
{
	return m_value;
}

}; //namespace rurt