/* fresnel.hpp
 *
 * contains the definition of the fresnel class, which represents
 * a general fresnel reflectance equation
 */

#ifndef RURT_FRESNEL_H
#define RURT_FRESNEL_H

#include "quickmath.hpp"
using namespace qm;

//-------------------------------------------//

namespace rurt
{

class Fresnel
{
public:
	virtual vec3 evaluate(float cosThetaI) const = 0;
};

} //namespace rurt

#endif //#ifndef RURT_FRESNEL_H