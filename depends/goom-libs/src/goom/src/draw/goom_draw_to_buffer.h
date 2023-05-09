#pragma once

#undef NO_LOGGING
#include "goom_config.h"
#include "goom_draw.h"
#include "goom_graphic.h"
#include "goom_logger.h"
#include "goom_types.h"
#include "point2d.h"

namespace GOOM
{
class GoomLogger;
}

namespace GOOM::DRAW
{

class GoomDrawToSingleBuffer : public IGoomDraw
{
public:
  GoomDrawToSingleBuffer(const Dimensions& dimensions, GoomLogger& goomLogger) noexcept;

  auto SetBuffer(PixelBuffer& buff) noexcept -> void;

  [[nodiscard]] auto GetPixel(const Point2dInt& point) const noexcept -> Pixel override;
  auto DrawPixelsUnblended(const Point2dInt& point, const MultiplePixels& colors) noexcept
      -> void override;

protected:
  auto DrawPixelsToDevice(const Point2dInt& point, const MultiplePixels& colors) noexcept
      -> void override;

private:
  [[maybe_unused]] GoomLogger* m_goomLogger;
  PixelBuffer* m_buffer{};
};

class GoomDrawToTwoBuffers : public IGoomDraw
{
public:
  GoomDrawToTwoBuffers(const Dimensions& dimensions, GoomLogger& goomLogger) noexcept;

  auto SetBuffers(PixelBuffer& buffer1, PixelBuffer& buffer2) noexcept -> void;

  [[nodiscard]] auto GetPixel(const Point2dInt& point) const noexcept -> Pixel override;
  auto DrawPixelsUnblended(const Point2dInt& point, const MultiplePixels& colors) noexcept
      -> void override;

protected:
  auto DrawPixelsToDevice(const Point2dInt& point, const MultiplePixels& colors) noexcept
      -> void override;

private:
  [[maybe_unused]] GoomLogger* m_goomLogger;
  PixelBuffer* m_buffer1{};
  PixelBuffer* m_buffer2{};
};

inline auto GoomDrawToSingleBuffer::SetBuffer(PixelBuffer& buff) noexcept -> void
{
  m_buffer = &buff;
}

inline auto GoomDrawToSingleBuffer::GetPixel(const Point2dInt& point) const noexcept -> Pixel
{
  Expects(m_buffer != nullptr);
  return (*m_buffer)(point.x, point.y);
}

inline auto GoomDrawToSingleBuffer::DrawPixelsUnblended(const Point2dInt& point,
                                                        const MultiplePixels& colors) noexcept
    -> void
{
  const auto buffPos = m_buffer->GetBuffPos(point.x, point.y);

  m_buffer->GetPixel(buffPos) = colors.color1;
}

inline auto GoomDrawToSingleBuffer::DrawPixelsToDevice(const Point2dInt& point,
                                                       const MultiplePixels& colors) noexcept
    -> void
{
  const auto buffPos = m_buffer->GetBuffPos(point.x, point.y);

  auto& pixel = m_buffer->GetPixel(buffPos);
  pixel       = GetBlendedPixel(pixel, GetIntBuffIntensity(), colors.color1, colors.color1.A());
}

inline auto GoomDrawToTwoBuffers::SetBuffers(PixelBuffer& buffer1, PixelBuffer& buffer2) noexcept
    -> void
{
  m_buffer1 = &buffer1;
  m_buffer2 = &buffer2;
}

inline auto GoomDrawToTwoBuffers::GetPixel(const Point2dInt& point) const noexcept -> Pixel
{
  return (*m_buffer1)(point.x, point.y);
}

inline auto GoomDrawToTwoBuffers::DrawPixelsUnblended(const Point2dInt& point,
                                                      const MultiplePixels& colors) noexcept -> void
{
  const auto buffPos = m_buffer1->GetBuffPos(point.x, point.y);

  m_buffer1->GetPixel(buffPos) = colors.color1;
  m_buffer2->GetPixel(buffPos) = colors.color2;
}

inline auto GoomDrawToTwoBuffers::DrawPixelsToDevice(const Point2dInt& point,
                                                     const MultiplePixels& colors) noexcept -> void
{
  const auto buffPos = m_buffer1->GetBuffPos(point.x, point.y);

  //  Expects(colors.color1.A() < MAX_ALPHA / 9);
  auto& pixel1 = m_buffer1->GetPixel(buffPos);
  pixel1       = GetBlendedPixel(pixel1, GetIntBuffIntensity(), colors.color1, colors.color1.A());
  if (pixel1.A() >= MAX_ALPHA / 9)
  {
    LogError(*m_goomLogger, "pixel1.A() = {}", pixel1.A());
  }
  //  Expects(pixel1.A() < MAX_ALPHA / 9);

  //  Expects(colors.color2.A() < MAX_ALPHA / 9);
  auto& pixel2 = m_buffer2->GetPixel(buffPos);
  pixel2       = GetBlendedPixel(pixel2, GetIntBuffIntensity(), colors.color2, colors.color2.A());
  if (pixel2.A() >= MAX_ALPHA / 9)
  {
    LogError(*m_goomLogger, "pixel2.A() = {}", pixel2.A());
  }
  //  Expects(pixel2.A() < MAX_ALPHA / 9);
}

} // namespace GOOM::DRAW
