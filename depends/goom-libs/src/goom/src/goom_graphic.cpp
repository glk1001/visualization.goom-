module;

#include <cstddef>

module Goom.Lib.GoomGraphic;

import Goom.Lib.AssertUtils;
import Goom.Lib.GoomTypes;

namespace GOOM
{

PixelBuffer::PixelBuffer(const Dimensions& dimensions) noexcept
  : m_width{dimensions.GetWidth()}, m_height{dimensions.GetHeight()}
{
}

PixelBuffer::PixelBuffer(const Buffer& buffer, const Dimensions& dimensions) noexcept
  : m_width{dimensions.GetWidth()}, m_height{dimensions.GetHeight()}, m_buff{buffer}
{
}

auto PixelBuffer::SetPixelBuffer(const Buffer& buffer) noexcept -> void
{
  Expects(buffer.size() == static_cast<size_t>(m_width) * static_cast<size_t>(m_height));

  m_buff = buffer;
}

auto PixelBuffer::SetPixelBuffer(const Buffer& buffer,
                                 const Dimensions& dimensions) noexcept -> void
{
  Expects(buffer.size() == dimensions.GetSize());

  m_width  = dimensions.GetWidth();
  m_height = dimensions.GetHeight();
  m_buff   = buffer;
}

} // namespace GOOM
