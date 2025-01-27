/* fr_texture_constant.hpp
 *
 * contains a definition for a constant-value texture
 */

#ifndef FR_TEXTURE_CONSTANT_H
#define FR_TEXTURE_CONSTANT_H

#include "../fr_texture.hpp"

//-------------------------------------------//

namespace fr
{

template<typename T>
class TextureConstant : public Texture<T>
{
public:
	TextureConstant(const T& value) : m_value(value) {}

	T evaluate(const IntersectionInfo& hitInfo) const override { return m_value; }

private:
	T m_value;
};

}; //namespace fr

#endif //#ifndef FR_TEXTURE_CONSTANT_H