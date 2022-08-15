#pragma once

#include "color/color_utils.h"
#include "draw_methods.h"
#include "goom_graphic.h"
#include "goom_types.h"
#include "point2d.h"
#include "utils/parallel_utils.h"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace GOOM
{

namespace UTILS::GRAPHICS
{
class ImageBitmap;
}

namespace DRAW
{

using MultiplePixels = std::vector<Pixel>;

[[nodiscard]] auto ReversePixels(const MultiplePixels& colors) noexcept -> MultiplePixels;
[[nodiscard]] auto GetMainColor(const MultiplePixels& colors) noexcept -> Pixel;
[[nodiscard]] auto GetLowColor(const MultiplePixels& colors) noexcept -> Pixel;

class IGoomDraw
{
  // Use the following to check it's efficient to pass Point2dInts by value.
  static constexpr size_t MAX_NUM_VOID_PTRS = 3;
  static_assert(sizeof(Point2dInt) < (MAX_NUM_VOID_PTRS * sizeof(void*)));

public:
  IGoomDraw() noexcept = delete;
  explicit IGoomDraw(const Dimensions& dimensions);
  IGoomDraw(const IGoomDraw&) noexcept           = delete;
  IGoomDraw(IGoomDraw&&) noexcept                = delete;
  virtual ~IGoomDraw() noexcept                  = default;
  auto operator=(const IGoomDraw&) -> IGoomDraw& = delete;
  auto operator=(IGoomDraw&&) -> IGoomDraw&      = delete;

  [[nodiscard]] auto GetScreenDimensions() const -> const Dimensions&;
  [[nodiscard]] auto GetScreenWidth() const -> uint32_t ;
  [[nodiscard]] auto GetScreenHeight() const -> uint32_t ;

  [[nodiscard]] auto GetParallel() const -> GOOM::UTILS::Parallel&;

  [[nodiscard]] auto GetBuffIntensity() const -> float;
  void SetBuffIntensity(float val);

  using BlendPixelFunc =
      std::function<Pixel(const Pixel& oldColor, const Pixel& newColor, uint32_t intBuffIntensity)>;
  void SetBlendPixelFunc(const BlendPixelFunc& func);
  void SetDefaultBlendPixelFunc();

  void Circle(Point2dInt point, int radius, const Pixel& color);
  void Circle(Point2dInt point, int radius, const MultiplePixels& colors);

  void Line(Point2dInt point1, Point2dInt point2, const Pixel& color, uint8_t thickness);
  void Line(Point2dInt point1, Point2dInt point2, const MultiplePixels& colors, uint8_t thickness);

  using GetBitmapColorFunc = std::function<Pixel(size_t x, size_t y, const Pixel& imageColor)>;
  void Bitmap(Point2dInt centre,
              const UTILS::GRAPHICS::ImageBitmap& bitmap,
              const GetBitmapColorFunc& getColor);
  void Bitmap(Point2dInt centre,
              const UTILS::GRAPHICS::ImageBitmap& bitmap,
              const std::vector<GetBitmapColorFunc>& getColors);

  void DrawPixels(Point2dInt point, const MultiplePixels& colors);
  void DrawPixelsClipped(Point2dInt point, const MultiplePixels& colors);

  [[nodiscard]] virtual auto GetPixel(Point2dInt point) const -> Pixel             = 0;
  virtual void DrawPixelsUnblended(Point2dInt point, const MultiplePixels& colors) = 0;

protected:
  [[nodiscard]] auto GetIntBuffIntensity() const -> uint32_t;

  // Use the following to set the final pixel in the buffer.
  [[nodiscard]] auto GetBlendedPixel(const Pixel& oldColor,
                                     const Pixel& newColor,
                                     uint32_t intBuffIntensity) const -> Pixel;

  virtual void DrawPixelsToDevice(Point2dInt point,
                                  const MultiplePixels& colors,
                                  uint32_t intBuffIntensity) = 0;

private:
  const Dimensions m_dimensions;
  DrawMethods m_drawMethods;
  void DrawPixelsToDevice(Point2dInt point, const MultiplePixels& colors);

  BlendPixelFunc m_blendPixelFunc{};
  [[nodiscard]] static auto ColorAddBlendPixel(const Pixel& oldColor,
                                               const Pixel& newColor,
                                               uint32_t intBuffIntensity) -> Pixel;
  static constexpr float DEFAULT_BUFF_INTENSITY = 0.5F;
  float m_buffIntensity                         = DEFAULT_BUFF_INTENSITY;
  uint32_t m_intBuffIntensity{};
  mutable GOOM::UTILS::Parallel m_parallel{-1}; // max cores - 1
};

inline auto ReversePixels(const MultiplePixels& colors) noexcept -> MultiplePixels
{
  return {colors.at(1), colors.at(0)};
}

inline auto GetMainColor(const MultiplePixels& colors) noexcept -> Pixel
{
  return colors.at(0);
}

inline auto GetLowColor(const MultiplePixels& colors) noexcept -> Pixel
{
  return colors.at(1);
}

inline auto IGoomDraw::GetScreenDimensions() const -> const Dimensions&
{
  return m_dimensions;
}

inline auto IGoomDraw::GetScreenWidth() const -> uint32_t
{
  return GetScreenDimensions().GetWidth();
}

inline auto IGoomDraw::GetScreenHeight() const -> uint32_t
{
  return GetScreenDimensions().GetHeight();
}

inline auto IGoomDraw::GetBuffIntensity() const -> float
{
  return m_buffIntensity;
}

inline void IGoomDraw::SetBuffIntensity(const float val)
{
  m_buffIntensity = val;
  m_intBuffIntensity =
      static_cast<uint32_t>(std::round(channel_limits<float>::max() * m_buffIntensity));
}

inline auto IGoomDraw::GetIntBuffIntensity() const -> uint32_t
{
  return m_intBuffIntensity;
}

inline void IGoomDraw::SetBlendPixelFunc(const BlendPixelFunc& func)
{
  m_blendPixelFunc = func;
}

inline void IGoomDraw::SetDefaultBlendPixelFunc()
{
  m_blendPixelFunc =
      [](const Pixel& oldColor, const Pixel& newColor, const uint32_t intBuffIntensity)
  { return ColorAddBlendPixel(oldColor, newColor, intBuffIntensity); };
}

inline auto IGoomDraw::GetBlendedPixel(const Pixel& oldColor,
                                       const Pixel& newColor,
                                       const uint32_t intBuffIntensity) const -> Pixel
{
  return m_blendPixelFunc(oldColor, newColor, intBuffIntensity);
}

inline auto IGoomDraw::ColorAddBlendPixel(const Pixel& oldColor,
                                          const Pixel& newColor,
                                          const uint32_t intBuffIntensity) -> Pixel
{
  return COLOR::GetColorAdd(oldColor, COLOR::GetBrighterColorInt(intBuffIntensity, newColor));
}

inline auto IGoomDraw::GetParallel() const -> GOOM::UTILS::Parallel&
{
  return m_parallel;
}

inline void IGoomDraw::Circle(const Point2dInt point, const int radius, const Pixel& color)
{
  Circle(point, radius, MultiplePixels{color});
}

inline void IGoomDraw::Circle(const Point2dInt point,
                              const int radius,
                              const MultiplePixels& colors)
{
  m_drawMethods.DrawCircle(point.x, point.y, radius, colors);
}

inline void IGoomDraw::Line(const Point2dInt point1,
                            const Point2dInt point2,
                            const Pixel& color,
                            const uint8_t thickness)
{
  Line(point1, point2, MultiplePixels{color}, thickness);
}

inline void IGoomDraw::Line(const Point2dInt point1,
                            const Point2dInt point2,
                            const MultiplePixels& colors,
                            const uint8_t thickness)
{
  m_drawMethods.DrawLine(point1.x, point1.y, point2.x, point2.y, colors, thickness);
}

inline void IGoomDraw::Bitmap(const Point2dInt centre,
                              const UTILS::GRAPHICS::ImageBitmap& bitmap,
                              const GetBitmapColorFunc& getColor)
{
  // WARNING undefined behaviour - GCC 11 does not like passing just '{getColor}'.
  Bitmap(centre, bitmap, std::vector<GetBitmapColorFunc>{getColor});
}

inline void IGoomDraw::DrawPixels(const Point2dInt point, const MultiplePixels& colors)
{
  m_drawMethods.DrawPixels(point.x, point.y, colors);
}

inline void IGoomDraw::DrawPixelsClipped(const Point2dInt point, const MultiplePixels& colors)
{
  if ((0 < point.x) or (0 < point.y) or
      (static_cast<uint32_t>(point.x) >= m_dimensions.GetWidth()) or
      (static_cast<uint32_t>(point.y) >= m_dimensions.GetHeight()))
  {
    return;
  }

  m_drawMethods.DrawPixels(point.x, point.y, colors);
}

} // namespace DRAW
} // namespace GOOM
