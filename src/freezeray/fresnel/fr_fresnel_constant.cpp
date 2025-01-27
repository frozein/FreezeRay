#include "freezeray/fresnel/fr_fresnel_constant.hpp"
#include "freezeray/fr_globals.hpp"

//-------------------------------------------//

namespace fr
{

FresnelConstant::FresnelConstant(const vec3& value) :
	m_value(value)
{

}

vec3 FresnelConstant::evaluate(float cosnThetaI) const
{
	return m_value;
}

}; //namespace fr