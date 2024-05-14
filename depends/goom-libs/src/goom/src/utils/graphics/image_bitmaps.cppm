module;

#include "goom/goom_graphic.h"
#include "goom/goom_types.h"

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <tuple>
#include <vector>

export module Goom.Utils.Graphics.ImageBitmaps;

export namespace GOOM::UTILS::GRAPHICS
{

class ImageBitmap
{
public:
  ImageBitmap() noexcept = default;
  explicit ImageBitmap(const std::string& imageFilename);

  auto Load(const std::string& imageFilename) -> void;

  [[nodiscard]] auto GetWidth() const noexcept -> uint32_t;
  [[nodiscard]] auto GetHeight() const noexcept -> uint32_t;

  [[nodiscard]] auto GetIntWidth() const noexcept -> int32_t;
  [[nodiscard]] auto GetIntHeight() const noexcept -> int32_t;

  auto operator()(size_t x, size_t y) const noexcept -> Pixel;

protected:
  struct RGB
  {
    uint8_t red   = 0;
    uint8_t green = 0;
    uint8_t blue  = 0;
    uint8_t alpha = 0;
  };

  [[nodiscard]] auto GetBuffer() const noexcept -> std::span<const RGB>;

private:
  uint32_t m_width{};
  uint32_t m_height{};
  std::vector<RGB> m_owningBuff{};
  std::string m_filename{};
  [[nodiscard]] auto GetRGBImage() const -> std::tuple<uint8_t*, int32_t, int32_t, int32_t>;
  auto SetPixel(size_t x, size_t y, const RGB& pixel) noexcept -> void;
  auto Resize(const Dimensions& dimensions) noexcept -> void;
};

inline ImageBitmap::ImageBitmap(const std::string& imageFilename)
{
  Load(imageFilename);
}

inline auto ImageBitmap::GetWidth() const noexcept -> uint32_t
{
  return m_width;
}

inline auto ImageBitmap::GetHeight() const noexcept -> uint32_t
{
  return m_height;
}

inline auto ImageBitmap::GetIntWidth() const noexcept -> int32_t
{
  return static_cast<int32_t>(m_width);
}

inline auto ImageBitmap::GetIntHeight() const noexcept -> int32_t
{
  return static_cast<int32_t>(m_height);
}

inline auto ImageBitmap::operator()(const size_t x, const size_t y) const noexcept -> Pixel
{
  const auto& pixel = m_owningBuff.at((y * m_width) + x);
  return Pixel{
      {pixel.red, pixel.green, pixel.blue, ToPixelAlpha(pixel.alpha)}
  };
}

inline auto ImageBitmap::GetBuffer() const noexcept -> std::span<const RGB>
{
  return std::span<const RGB>{m_owningBuff};
}

} // namespace GOOM::UTILS::GRAPHICS
