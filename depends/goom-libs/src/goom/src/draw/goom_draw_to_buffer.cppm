module;

#include "goom/goom_config.h"
#include "goom/goom_logger.h"

export module Goom.Draw.GoomDrawToBuffer;

import Goom.Draw.GoomDrawBase;
import Goom.Lib.GoomGraphic;
import Goom.Lib.GoomTypes;
import Goom.Lib.Point2d;

export namespace GOOM::DRAW
{

class GoomDrawToSingleBuffer : public IGoomDraw
{
public:
  GoomDrawToSingleBuffer(const Dimensions& dimensions, GoomLogger& goomLogger) noexcept;

  [[nodiscard]] auto GetBuffer() const noexcept -> const PixelBuffer&;
  [[nodiscard]] auto GetBuffer() noexcept -> PixelBuffer&;
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

  [[nodiscard]] auto GetBuffer1() const noexcept -> const PixelBuffer&;
  [[nodiscard]] auto GetBuffer1() noexcept -> PixelBuffer&;
  [[nodiscard]] auto GetBuffer2() const noexcept -> const PixelBuffer&;
  [[nodiscard]] auto GetBuffer2() noexcept -> PixelBuffer&;
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

inline auto GoomDrawToSingleBuffer::GetBuffer() const noexcept -> const PixelBuffer&
{
  return *m_buffer;
}

inline auto GoomDrawToSingleBuffer::GetBuffer() noexcept -> PixelBuffer&
{
  return *m_buffer;
}

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
  (*m_buffer)(point.x, point.y) = colors.color1;
}

inline auto GoomDrawToSingleBuffer::DrawPixelsToDevice(const Point2dInt& point,
                                                       const MultiplePixels& colors) noexcept
    -> void
{
  const auto buffPos = m_buffer->GetBuffPos(point.x, point.y);

  auto& pixel = m_buffer->GetPixel(buffPos);
  pixel       = GetBlendedPixel(pixel, GetIntBuffIntensity(), colors.color1, colors.color1.A());
}

inline auto GoomDrawToTwoBuffers::GetBuffer1() const noexcept -> const PixelBuffer&
{
  return *m_buffer1;
}

inline auto GoomDrawToTwoBuffers::GetBuffer1() noexcept -> PixelBuffer&
{
  return *m_buffer1;
}

inline auto GoomDrawToTwoBuffers::GetBuffer2() const noexcept -> const PixelBuffer&
{
  return *m_buffer2;
}

inline auto GoomDrawToTwoBuffers::GetBuffer2() noexcept -> PixelBuffer&
{
  return *m_buffer2;
}

inline auto GoomDrawToTwoBuffers::SetBuffers(PixelBuffer& buffer1, PixelBuffer& buffer2) noexcept
    -> void
{
  Expects(buffer1.GetWidth() == GetDimensions().GetWidth());
  Expects(buffer2.GetWidth() == GetDimensions().GetWidth());

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

  auto& pixel1 = m_buffer1->GetPixel(buffPos);
  pixel1       = GetBlendedPixel(pixel1, GetIntBuffIntensity(), colors.color1, colors.color1.A());

  auto& pixel2 = m_buffer2->GetPixel(buffPos);
  pixel2       = GetBlendedPixel(pixel2, GetIntBuffIntensity(), colors.color2, colors.color2.A());
}

} // namespace GOOM::DRAW
