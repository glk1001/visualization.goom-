#include "goom_graphic.h"

namespace GOOM
{

PixelBuffer::PixelBuffer(const Dimensions& dimensions) noexcept
  : m_width{dimensions.GetWidth()}, m_height{dimensions.GetHeight()}, m_buff(dimensions.GetSize())
{
}

} // namespace GOOM
