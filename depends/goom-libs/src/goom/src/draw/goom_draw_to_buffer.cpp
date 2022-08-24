#include "goom_draw_to_buffer.h"

#include "goom_config.h"
#include "goom_graphic.h"

#include <cstdint>
#include <vector>

namespace GOOM::DRAW
{

GoomDrawToBuffer::GoomDrawToBuffer(const Dimensions& dimensions) : IGoomDraw{dimensions}
{
}

GoomDrawToBuffer::~GoomDrawToBuffer() noexcept = default;

void GoomDrawToBuffer::SetBuffers(const std::vector<PixelBuffer*>& buffs)
{
  Expects(buffs.size() > 0);
  m_multipleBuffers = buffs;
  m_multipleBuffer0 = m_multipleBuffers[0];
  m_numBuffers      = m_multipleBuffers.size();
}

auto GoomDrawToBuffer::GetPixel(const Point2dInt point) const -> Pixel
{
  Expects(not m_multipleBuffers.empty());
  Expects(m_multipleBuffer0 != nullptr);
  return (*m_multipleBuffer0)(point.x, point.y);
}

void GoomDrawToBuffer::DrawPixelsUnblended(const Point2dInt point, const MultiplePixels& colors)
{
  const auto buffPos = m_multipleBuffer0->GetBuffPos(point.x, point.y);

  for (auto i = 0U; i < m_numBuffers; ++i)
  {
    m_multipleBuffers[i]->GetPixel(buffPos) = colors[i];
  }
}

void GoomDrawToBuffer::DrawPixelsToDevice(const Point2dInt point,
                                          const MultiplePixels& colors,
                                          const uint32_t intBuffIntensity)
{
  const auto buffPos = m_multipleBuffer0->GetBuffPos(point.x, point.y);

  for (auto i = 0U; i < m_numBuffers; ++i)
  {
    auto& pixel = m_multipleBuffers[i]->GetPixel(buffPos);
    pixel       = GetBlendedPixel(pixel, colors[i], intBuffIntensity);
  }
}

} // namespace GOOM::DRAW
