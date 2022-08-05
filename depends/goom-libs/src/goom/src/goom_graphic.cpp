#include "goom_graphic.h"

#include <format>
#include <string>

namespace GOOM
{

auto Pixel::ToString() const -> std::string
{
  return std20::format("({}, {}, {}, {})", R(), G(), B(), A());
}

PixelBuffer::PixelBuffer(const Dimensions& dimensions) noexcept
  : m_width{dimensions.GetWidth()},
    m_height{dimensions.GetHeight()},
    m_buff(dimensions.GetSize())
{
}

auto PixelBuffer::Resize(const Dimensions& dimensions) noexcept -> void
{
  m_width  = dimensions.GetWidth();
  m_height = dimensions.GetHeight();
  m_xMax   = m_width - 1;
  m_yMax   = m_height - 1;
  m_buff.resize(dimensions.GetSize());
}

} // namespace GOOM
