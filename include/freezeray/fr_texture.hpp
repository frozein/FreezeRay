/* fr_texture.hpp
 *
 * contains the definition of the texture class, which 
 * represent an abstract parameter-space sampler for obtaining shading attributes
 */

#ifndef FR_TEXTURE_H
#define FR_TEXTURE_H

#include "fr_raycast_info.hpp"

//-------------------------------------------//

namespace fr
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

}; //namespace fr

#endif //#ifndef FR_TEXTURE_H