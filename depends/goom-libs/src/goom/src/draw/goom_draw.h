#pragma once

#include "color/colorutils.h"
#include "draw_methods.h"
#include "goom_graphic.h"
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

class IGoomDraw
{
public:
  IGoomDraw() noexcept = delete;
  IGoomDraw(uint32_t screenWidth, uint32_t screenHeight);
  IGoomDraw(const IGoomDraw&) noexcept = delete;
  IGoomDraw(IGoomDraw&&) noexcept = delete;
  virtual ~IGoomDraw() noexcept = default;
  auto operator=(const IGoomDraw&) -> IGoomDraw& = delete;
  auto operator=(IGoomDraw&&) -> IGoomDraw& = delete;

  [[nodiscard]] auto GetScreenWidth() const -> uint32_t;
  [[nodiscard]] auto GetScreenHeight() const -> uint32_t;

  [[nodiscard]] auto GetParallel() const -> GOOM::UTILS::Parallel&;

  [[nodiscard]] auto GetBuffIntensity() const -> float;
  void SetBuffIntensity(float val);

  using BlendPixelFunc =
      std::function<Pixel(const Pixel& oldColor, const Pixel& newColor, uint32_t intBuffIntensity)>;
  void SetBlendPixelFunc(const BlendPixelFunc& func);
  void SetDefaultBlendPixelFunc();

  void Circle(int x0, int y0, int radius, const Pixel& color);
  void Circle(int x0, int y0, int radius, const std::vector<Pixel>& colors);

  void Line(int x1, int y1, int x2, int y2, const Pixel& color, uint8_t thickness);
  void Line(int x1, int y1, int x2, int y2, const std::vector<Pixel>& colors, uint8_t thickness);

  using GetBitmapColorFunc = std::function<Pixel(size_t x, size_t y, const Pixel& imageColor)>;
  void Bitmap(int xCentre,
              int yCentre,
              const UTILS::GRAPHICS::ImageBitmap& bitmap,
              const GetBitmapColorFunc& getColor);
  void Bitmap(int xCentre,
              int yCentre,
              const UTILS::GRAPHICS::ImageBitmap& bitmap,
              const std::vector<GetBitmapColorFunc>& getColors);

  void DrawPixels(int32_t x, int32_t y, const std::vector<Pixel>& colors);

  [[nodiscard]] virtual auto GetPixel(int32_t x, int32_t y) const -> Pixel = 0;
  virtual void DrawPixelsUnblended(int32_t x, int32_t y, const std::vector<Pixel>& colors) = 0;

protected:
  [[nodiscard]] auto GetIntBuffIntensity() const -> uint32_t;

  // Use the following to set the final pixel in the buffer.
  [[nodiscard]] auto GetBlendedPixel(const Pixel& oldColor,
                                     const Pixel& newColor,
                                     uint32_t intBuffIntensity) const -> Pixel;

  virtual void DrawPixelsToDevice(int32_t x,
                                  int32_t y,
                                  const std::vector<Pixel>& colors,
                                  uint32_t intBuffIntensity) = 0;

private:
  const uint32_t m_screenWidth;
  const uint32_t m_screenHeight;
  const DrawMethods m_drawMethods;
  void DrawPixelsToDevice(int32_t x, int32_t y, const std::vector<Pixel>& colors);

  BlendPixelFunc m_blendPixelFunc{};
  [[nodiscard]] static auto ColorAddBlendPixel(const Pixel& oldColor,
                                               const Pixel& newColor,
                                               uint32_t intBuffIntensity) -> Pixel;
  static constexpr float DEFAULT_BUFF_INTENSITY = 0.5F;
  float m_buffIntensity = DEFAULT_BUFF_INTENSITY;
  uint32_t m_intBuffIntensity;
  mutable GOOM::UTILS::Parallel m_parallel;
};

inline auto IGoomDraw::GetScreenWidth() const -> uint32_t
{
  return m_screenWidth;
}

inline auto IGoomDraw::GetScreenHeight() const -> uint32_t
{
  return m_screenHeight;
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

inline void IGoomDraw::Circle(const int x0, const int y0, const int radius, const Pixel& color)
{
  Circle(x0, y0, radius, std::vector<Pixel>{color});
}

inline void IGoomDraw::Circle(const int x0,
                              const int y0,
                              const int radius,
                              const std::vector<Pixel>& colors)
{
  m_drawMethods.DrawCircle(x0, y0, radius, colors);
}

inline void IGoomDraw::Line(const int x1,
                            const int y1,
                            const int x2,
                            const int y2,
                            const Pixel& color,
                            const uint8_t thickness)
{
  Line(x1, y1, x2, y2, std::vector<Pixel>{color}, thickness);
}

inline void IGoomDraw::Line(const int x1,
                            const int y1,
                            const int x2,
                            const int y2,
                            const std::vector<Pixel>& colors,
                            const uint8_t thickness)
{
  m_drawMethods.DrawLine(x1, y1, x2, y2, colors, thickness);
}

inline void IGoomDraw::Bitmap(const int xCentre,
                              const int yCentre,
                              const UTILS::GRAPHICS::ImageBitmap& bitmap,
                              const GetBitmapColorFunc& getColor)
{
  // WARNING undefined behaviour - GCC 11 does not like passing just '{getColor}'.
  Bitmap(xCentre, yCentre, bitmap, std::vector<GetBitmapColorFunc>{getColor});
}

inline void IGoomDraw::DrawPixels(const int32_t x,
                                  const int32_t y,
                                  const std::vector<Pixel>& colors)
{
  m_drawMethods.DrawPixels(x, y, colors);
}

} // namespace DRAW
} // namespace GOOM
