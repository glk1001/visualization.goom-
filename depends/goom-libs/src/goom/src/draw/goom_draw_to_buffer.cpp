#include "goom_draw_to_buffer.h"

#include "color/colorutils.h"
#include "goom_graphic.h"

#undef NDEBUG
#include <cassert>
#include <cstdint>
#include <vector>

namespace GOOM::DRAW
{

GoomDrawToBuffer::GoomDrawToBuffer(const uint32_t screenWidth, const uint32_t screenHeight)
  : IGoomDraw{screenWidth, screenHeight,
              [&](const int32_t x, const int32_t y, const std::vector<Pixel>& newColors)
              { DrawPixels(m_multipleBuffers, x, y, newColors, GetIntBuffIntensity()); }}
{
}

auto GoomDrawToBuffer::GetPixel(const int32_t x, const int32_t y) const -> Pixel
{
  assert(!m_multipleBuffers.empty());
  assert(m_multipleBuffers[0] != nullptr);
  return (*m_multipleBuffers[0])(static_cast<size_t>(x), static_cast<size_t>(y));
}

void GoomDrawToBuffer::DrawPixelsUnblended(const int32_t x,
                                           const int32_t y,
                                           const std::vector<Pixel>& colors)
{
  for (size_t i = 0; i < m_multipleBuffers.size(); ++i)
  {
    (*m_multipleBuffers[i])(static_cast<size_t>(x), static_cast<size_t>(y)) = colors[i];
  }
}

void GoomDrawToBuffer::DrawPixels(const std::vector<PixelBuffer*>& buffs,
                                  const int32_t x,
                                  const int32_t y,
                                  const std::vector<Pixel>& colors,
                                  const uint32_t intBuffIntensity)
{
  for (size_t i = 0; i < colors.size(); ++i)
  {
    Pixel& pixel = (*buffs[i])(static_cast<size_t>(x), static_cast<size_t>(y));
    pixel = GetBlendedPixel(pixel, colors[i], intBuffIntensity);
  }
}

} // namespace GOOM::DRAW
