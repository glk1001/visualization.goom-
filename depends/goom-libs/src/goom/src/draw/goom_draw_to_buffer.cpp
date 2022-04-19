#include "goom_draw_to_buffer.h"

#include "goom_graphic.h"

#undef NDEBUG
#include <cassert>
#include <cstdint>
#include <vector>

namespace GOOM::DRAW
{

GoomDrawToBuffer::GoomDrawToBuffer(const uint32_t screenWidth, const uint32_t screenHeight)
  : IGoomDraw{screenWidth, screenHeight}
{
}

auto GoomDrawToBuffer::GetPixel(const Point2dInt point) const -> Pixel
{
  assert(!m_multipleBuffers.empty());
  assert(m_multipleBuffers[0] != nullptr);
  return (*m_multipleBuffers[0])(static_cast<size_t>(point.x), static_cast<size_t>(point.y));
}

void GoomDrawToBuffer::DrawPixelsUnblended(const Point2dInt point, const std::vector<Pixel>& colors)
{
  for (size_t i = 0; i < m_numBuffers; ++i)
  {
    (*m_multipleBuffers[i])(static_cast<size_t>(point.x), static_cast<size_t>(point.y)) = colors[i];
  }
}

void GoomDrawToBuffer::DrawPixelsToDevice(const Point2dInt point,
                                          const std::vector<Pixel>& colors,
                                          const uint32_t intBuffIntensity)
{
  for (size_t i = 0; i < m_numBuffers; ++i)
  {
    Pixel& pixel =
        (*m_multipleBuffers[i])(static_cast<size_t>(point.x), static_cast<size_t>(point.y));
    pixel = GetBlendedPixel(pixel, colors[i], intBuffIntensity);
  }
}

} // namespace GOOM::DRAW
