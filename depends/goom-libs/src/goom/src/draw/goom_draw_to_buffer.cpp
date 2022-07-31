#include "goom_draw_to_buffer.h"

#include "goom_config.h"
#include "goom_graphic.h"

#include <cstdint>
#include <vector>

namespace GOOM::DRAW
{

GoomDrawToBuffer::GoomDrawToBuffer(const uint32_t screenWidth, const uint32_t screenHeight)
  : IGoomDraw{screenWidth, screenHeight}
{
}

GoomDrawToBuffer::~GoomDrawToBuffer() noexcept = default;

void GoomDrawToBuffer::SetBuffers(const std::vector<PixelBuffer*>& buffs)
{
  m_multipleBuffers = buffs;
  m_numBuffers      = m_multipleBuffers.size();
}

auto GoomDrawToBuffer::GetPixel(const Point2dInt point) const -> Pixel
{
  Expects(not m_multipleBuffers.empty());
  Expects(m_multipleBuffers[0] != nullptr);
  return (*m_multipleBuffers[0])(static_cast<size_t>(point.x), static_cast<size_t>(point.y));
}

void GoomDrawToBuffer::DrawPixelsUnblended(const Point2dInt point, const MultiplePixels& colors)
{
  for (auto i = 0U; i < m_numBuffers; ++i)
  {
    (*m_multipleBuffers[i])(static_cast<size_t>(point.x), static_cast<size_t>(point.y)) = colors[i];
  }
}

void GoomDrawToBuffer::DrawPixelsToDevice(const Point2dInt point,
                                          const MultiplePixels& colors,
                                          const uint32_t intBuffIntensity)
{
  for (auto i = 0U; i < m_numBuffers; ++i)
  {
    auto& pixel =
        (*m_multipleBuffers[i])(static_cast<size_t>(point.x), static_cast<size_t>(point.y));
    pixel = GetBlendedPixel(pixel, colors[i], intBuffIntensity);
  }
}

} // namespace GOOM::DRAW
