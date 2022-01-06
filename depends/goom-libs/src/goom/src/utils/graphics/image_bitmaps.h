#pragma once

#include "goom/goom_graphic.h"

#include <cstdint>

namespace GOOM::UTILS
{

class ImageBitmap
{
public:
  ImageBitmap() noexcept {} // Older visual studio does not like '= default;'
  explicit ImageBitmap(std::string imageFilename);

  void Load(std::string imageFilename);

  [[nodiscard]] auto GetWidth() const -> uint32_t;
  [[nodiscard]] auto GetHeight() const -> uint32_t;

  auto operator()(size_t x, size_t y) const -> Pixel;

private:
  struct RGB
  {
    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;
    uint8_t alpha = 0;
  };
  uint32_t m_width{};
  uint32_t m_height{};
  using Buffer = std::vector<RGB>;
  Buffer m_buff{};
  std::string m_filename{};
  void SetPixel(size_t x, size_t y, const RGB& pixel);
  void Resize(size_t width, size_t height);
};

inline ImageBitmap::ImageBitmap(std::string imageFilename)
{
  Load(std::move(imageFilename));
}

inline auto ImageBitmap::GetWidth() const -> uint32_t
{
  return m_width;
}

inline auto ImageBitmap::GetHeight() const -> uint32_t
{
  return m_height;
}

inline auto ImageBitmap::operator()(const size_t x, const size_t y) const -> Pixel
{
  const RGB& pixel = m_buff.at((y * m_width) + x);
  return Pixel{{pixel.red, pixel.green, pixel.blue, pixel.alpha}};
}

} // namespace GOOM::UTILS
