/* fr_fresnel_constant.hpp
 *
 * contains a definition for a fresnel function returning a constant value
 */

#ifndef FR_FRESNEL_CONSTANT_H
#define FR_FRESNEL_CONSTANT_H

#include "../fr_fresnel.hpp"

//-------------------------------------------//

namespace fr
{

class FresnelConstant : public Fresnel
{
public:
	FresnelConstant(const vec3& value);

	vec3 evaluate(float cosThetaI) const override;

private:
	vec3 m_value;
};

} //namespcae fr

#endif //#ifndef FR_FRESNEL_CONSTANT_H