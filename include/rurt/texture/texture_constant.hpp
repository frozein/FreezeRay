/* texture_constant.hpp
 *
 * contains a definition for a constant-value texture
 */

#ifndef RURT_TEXTURE_CONSTANT_H
#define RURT_TEXTURE_CONSTANT_H

#include "../texture.hpp"

//-------------------------------------------//

namespace rurt
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

}; //namespace rurt

#endif //#ifndef RURT_TEXTURE_CONSTANT_H