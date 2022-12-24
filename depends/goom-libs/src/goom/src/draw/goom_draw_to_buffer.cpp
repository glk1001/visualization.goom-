#undef NO_LOGGING

#include "goom_draw_to_buffer.h"

#include "goom_config.h"
#include "goom_graphic.h"
#include "goom_logger.h"

#include <cstdint>
#include <vector>

namespace GOOM::DRAW
{

GoomDrawToBuffer::GoomDrawToBuffer(const Dimensions& dimensions, GoomLogger& goomLogger) noexcept
  : IGoomDraw{dimensions}, m_goomLogger{goomLogger}
{
}

GoomDrawToBuffer::~GoomDrawToBuffer() noexcept = default;

auto GoomDrawToBuffer::SetBuffers(const std::vector<PixelBuffer*>& buffs) noexcept -> void
{
  Expects(buffs.size() > 0);
  m_multipleBuffers = buffs;
  m_multipleBuffer0 = m_multipleBuffers[0];
  m_numBuffers      = m_multipleBuffers.size();
}

auto GoomDrawToBuffer::GetPixel(const Point2dInt point) const noexcept -> Pixel
{
  Expects(not m_multipleBuffers.empty());
  Expects(m_multipleBuffer0 != nullptr);
  return (*m_multipleBuffer0)(point.x, point.y);
}

auto GoomDrawToBuffer::DrawPixelsUnblended(const Point2dInt point,
                                           const MultiplePixels& colors) noexcept -> void
{
  const auto buffPos = m_multipleBuffer0->GetBuffPos(point.x, point.y);

  for (auto i = 0U; i < m_numBuffers; ++i)
  {
    m_multipleBuffers[i]->GetPixel(buffPos) = colors[i];
  }
}

auto GoomDrawToBuffer::DrawPixelsToDevice(const Point2dInt point,
                                          const MultiplePixels& colors,
                                          const uint32_t intBuffIntensity) noexcept -> void
{
  const auto buffPos = m_multipleBuffer0->GetBuffPos(point.x, point.y);

  for (auto i = 0U; i < m_numBuffers; ++i)
  {
    auto& pixel = m_multipleBuffers[i]->GetPixel(buffPos);
    if (750 < point.x and point.x < 850 and 400 < point.y and point.y < 550)
    {
      LogInfo(m_goomLogger,
              "point = {},{}, pixel = {},{},{}; newColor = {},{},{}, intBuffIntensity = {}",
              point.x,
              point.y,
              pixel.R(),
              pixel.G(),
              pixel.B(),
              colors[i].R(),
              colors[i].G(),
              colors[i].B(),
              intBuffIntensity);
    }
    pixel       = GetBlendedPixel(pixel, colors[i], intBuffIntensity);
    if (750 < point.x and point.x < 850 and 400 < point.y and point.y < 550)
    {
      LogInfo(m_goomLogger, "point = {},{}, blended pixel = {},{},{}",
              point.x, point.y, pixel.R(), pixel.G(), pixel.B());
    }
  }
}

} // namespace GOOM::DRAW
