/* texture.hpp
 *
 * contains the definition of the texture class, which 
 * represent an abstract parameter-space sampler for obtaining shading attributes
 */

#ifndef RURT_TEXTURE_H
#define RURT_TEXTURE_H

#include "raycast_info.hpp"

//-------------------------------------------//

namespace rurt
{

enum class TextureRepeatMode
{
	REPEAT,
	CLAMP_TO_EDGE
};

template<typename T>
class Texture
{
public:
	virtual T evaluate(const IntersectionInfo& hitInfo) const = 0;
};

}; //namespace rurt

#endif //#ifndef RURT_TEXTURE_H