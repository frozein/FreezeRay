/* fresnel_constant.hpp
 *
 * contains a definition for a fresnel function returning a constant value
 */

#ifndef RURT_FRESNEL_CONSTANT_H
#define RURT_FRESNEL_CONSTANT_H

#include "../fresnel.hpp"

//-------------------------------------------//

namespace rurt
{

class FresnelConstant : public Fresnel
{
public:
	FresnelConstant(const vec3& value);

	vec3 evaluate(float cosThetaI) const override;

private:
	vec3 m_value;
};

} //namespcae rurt

#endif //#ifndef RURT_FRESNEL_CONSTANT_H