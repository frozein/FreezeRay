/* fr_fresnel.hpp
 *
 * contains the definition of the fresnel class, which represents
 * a general fresnel reflectance equation
 */

#ifndef FR_FRESNEL_H
#define FR_FRESNEL_H

#include "quickmath.hpp"
using namespace qm;

//-------------------------------------------//

namespace fr
{

class Fresnel
{
public:
	virtual vec3 evaluate(float cosThetaI) const = 0;
};

} //namespace fr

#endif //#ifndef FR_FRESNEL_H