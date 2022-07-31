#include "goom_graphic.h"

#include <cstdint>
#include <format>
#include <string>

namespace GOOM
{

auto Pixel::ToString() const -> std::string
{
  return std20::format("({}, {}, {}, {})", R(), G(), B(), A());
}

PixelBuffer::PixelBuffer(const uint32_t width, const uint32_t height) noexcept
  : m_width{width},
    m_height{height},
    m_xMax{m_width - 1},
    m_yMax{m_height - 1},
    m_buff(static_cast<size_t>(m_width) * static_cast<size_t>(m_height))
{
}

auto PixelBuffer::Resize(const size_t width, const size_t height) noexcept -> void
{
  m_width  = static_cast<uint32_t>(width);
  m_height = static_cast<uint32_t>(height);
  m_xMax   = m_width - 1;
  m_yMax   = m_height - 1;
  m_buff.resize(static_cast<size_t>(m_width) * static_cast<size_t>(m_height));
}

} // namespace GOOM
