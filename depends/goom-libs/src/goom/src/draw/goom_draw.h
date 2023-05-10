#pragma once

#include "color/color_utils.h"
#include "goom_graphic.h"
#include "goom_types.h"
#include "point2d.h"
#include "utils/graphics/pixel_blend.h"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <vector>

namespace GOOM::DRAW
{

struct MultiplePixels
{
  Pixel color1;
  Pixel color2;
};

[[nodiscard]] auto MakePixels(const Pixel& mainColor, const Pixel& lowColor) noexcept
    -> MultiplePixels;
[[nodiscard]] auto ReversePixels(const MultiplePixels& colors) noexcept -> MultiplePixels;
[[nodiscard]] auto GetMainColor(const MultiplePixels& colors) noexcept -> Pixel;
[[nodiscard]] auto GetLowColor(const MultiplePixels& colors) noexcept -> Pixel;

class IGoomDraw
{
public:
  IGoomDraw() noexcept = delete;
  explicit IGoomDraw(const Dimensions& dimensions) noexcept;
  IGoomDraw(const IGoomDraw&) noexcept           = delete;
  IGoomDraw(IGoomDraw&&) noexcept                = delete;
  virtual ~IGoomDraw() noexcept                  = default;
  auto operator=(const IGoomDraw&) -> IGoomDraw& = delete;
  auto operator=(IGoomDraw&&) -> IGoomDraw&      = delete;

  [[nodiscard]] auto GetDimensions() const noexcept -> const Dimensions&;

  [[nodiscard]] auto GetBuffIntensity() const noexcept -> float;
  auto SetBuffIntensity(float val) noexcept -> void;

  using PixelBlendFunc = std::function<Pixel(const Pixel& bgndColor,
                                             uint32_t intBuffIntensity,
                                             const Pixel& fgndColor,
                                             PixelChannelType newAlpha)>;
  auto SetPixelBlendFunc(const PixelBlendFunc& func) noexcept -> void;
  auto SetDefaultPixelBlendFunc() noexcept -> void;

  auto DrawPixels(const Point2dInt& point, const MultiplePixels& colors) noexcept -> void;
  auto DrawClippedPixels(const Point2dInt& point, const MultiplePixels& colors) noexcept -> void;

  [[nodiscard]] virtual auto GetPixel(const Point2dInt& point) const noexcept -> Pixel = 0;
  virtual auto DrawPixelsUnblended(const Point2dInt& point, const MultiplePixels& colors) noexcept
      -> void = 0;

protected:
  [[nodiscard]] auto GetIntBuffIntensity() const noexcept -> uint32_t;

  // Use the following to set the final pixel in the buffer.
  [[nodiscard]] auto GetBlendedPixel(const Pixel& bgndColor,
                                     uint32_t intBuffIntensity,
                                     const Pixel& fgndColor,
                                     PixelChannelType newAlpha) const noexcept -> Pixel;

  virtual auto DrawPixelsToDevice(const Point2dInt& point, const MultiplePixels& colors) noexcept
      -> void = 0;

private:
  Dimensions m_dimensions;

  PixelBlendFunc m_pixelBlendFunc{};
  [[nodiscard]] static auto GetColorAddPixelBlend(const Pixel& bgndColor,
                                                  uint32_t intBuffIntensity,
                                                  const Pixel& fgndColor,
                                                  PixelChannelType newAlpha) noexcept -> Pixel;
  static constexpr float DEFAULT_BUFF_INTENSITY = 0.5F;
  float m_buffIntensity                         = DEFAULT_BUFF_INTENSITY;
  uint32_t m_intBuffIntensity                   = GetIntBuffIntensity(DEFAULT_BUFF_INTENSITY);
  [[nodiscard]] static auto GetIntBuffIntensity(float buffIntensity) noexcept -> uint32_t;
};

inline IGoomDraw::IGoomDraw(const Dimensions& dimensions) noexcept : m_dimensions{dimensions}
{
  SetBuffIntensity(m_buffIntensity);
  SetDefaultPixelBlendFunc();
}

inline auto MakePixels(const Pixel& mainColor, const Pixel& lowColor) noexcept -> MultiplePixels
{
  return {mainColor, lowColor};
}

inline auto ReversePixels(const MultiplePixels& colors) noexcept -> MultiplePixels
{
  return {colors.color2, colors.color1};
}

inline auto GetMainColor(const MultiplePixels& colors) noexcept -> Pixel
{
  return colors.color1;
}

inline auto GetLowColor(const MultiplePixels& colors) noexcept -> Pixel
{
  return colors.color2;
}

inline auto IGoomDraw::GetDimensions() const noexcept -> const Dimensions&
{
  return m_dimensions;
}

inline auto IGoomDraw::GetBuffIntensity() const noexcept -> float
{
  return m_buffIntensity;
}

inline auto IGoomDraw::SetBuffIntensity(const float val) noexcept -> void
{
  m_buffIntensity    = val;
  m_intBuffIntensity = GetIntBuffIntensity(m_buffIntensity);
}

inline auto IGoomDraw::GetIntBuffIntensity(const float buffIntensity) noexcept -> uint32_t
{
  return static_cast<uint32_t>(std::round(
      static_cast<float>(UTILS::GRAPHICS::CHANNEL_COLOR_SCALAR_DIVISOR) * buffIntensity));
}

inline auto IGoomDraw::GetIntBuffIntensity() const noexcept -> uint32_t
{
  return m_intBuffIntensity;
}

inline auto IGoomDraw::SetPixelBlendFunc(const PixelBlendFunc& func) noexcept -> void
{
  m_pixelBlendFunc = func;
}

inline auto IGoomDraw::SetDefaultPixelBlendFunc() noexcept -> void
{
  m_pixelBlendFunc = [](const Pixel& bgndColor,
                        const uint32_t intBuffIntensity,
                        const Pixel& fgndColor,
                        const PixelChannelType newAlpha)
  { return GetColorAddPixelBlend(bgndColor, intBuffIntensity, fgndColor, newAlpha); };
}

inline auto IGoomDraw::GetBlendedPixel(const Pixel& bgndColor,
                                       const uint32_t intBuffIntensity,
                                       const Pixel& fgndColor,
                                       const PixelChannelType newAlpha) const noexcept -> Pixel
{
  return m_pixelBlendFunc(bgndColor, intBuffIntensity, fgndColor, newAlpha);
}

inline auto IGoomDraw::GetColorAddPixelBlend(const Pixel& bgndColor,
                                             const uint32_t intBuffIntensity,
                                             const Pixel& fgndColor,
                                             const PixelChannelType newAlpha) noexcept -> Pixel
{
  return UTILS::GRAPHICS::GetColorAddPixelBlend(bgndColor, intBuffIntensity, fgndColor, newAlpha);
}

inline auto IGoomDraw::DrawPixels(const Point2dInt& point, const MultiplePixels& colors) noexcept
    -> void
{
  Expects(point.x >= 0);
  Expects(point.y >= 0);
  Expects(point.x < m_dimensions.GetIntWidth());
  Expects(point.y < m_dimensions.GetIntHeight());

  DrawPixelsToDevice(point, colors);
}

inline auto IGoomDraw::DrawClippedPixels(const Point2dInt& point,
                                         const MultiplePixels& colors) noexcept -> void
{
  if ((point.x < 0) or (point.y < 0) or (point.x >= m_dimensions.GetIntWidth()) or
      (point.y >= m_dimensions.GetIntHeight()))
  {
    return;
  }

  DrawPixelsToDevice(point, colors);
}

} // namespace GOOM::DRAW
