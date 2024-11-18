/* fresnel.hpp
 *
 * contains the definition of the fresnel class, which represents
 * a general fresnel reflectance equation
 */

#ifndef RURT_FRESNEL_H
#define RURT_FRESNEL_H

//-------------------------------------------//

namespace rurt
{

class Fresnel
{
public:
	virtual float evaluate(float cosThetaI) const = 0;
};

} //namespace rurt

#endif //#ifndef RURT_FRESNEL_H