#include "image_bitmaps.h"

#include "goom_types.h"

#include <cstddef>
#include <cstdint>
#include <exception>
#include <format> // NOLINT(misc-include-cleaner): Waiting for C++20.
#include <string>
#include <tuple>

#define STB_IMAGE_IMPLEMENTATION
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4068) // unknown pragma 'GCC'
#pragma warning(disable : 4296) // '>=': expression is always true
#endif
#include "../stb_image.h"
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <format> // NOLINT: Waiting to use C++20.
#include <stdexcept>

namespace GOOM::UTILS::GRAPHICS
{

auto ImageBitmap::Resize(const Dimensions& dimensions) noexcept -> void
{
  m_width  = dimensions.GetWidth();
  m_height = dimensions.GetHeight();
  m_buff.resize(static_cast<size_t>(m_width) * static_cast<size_t>(m_height));
}

inline auto ImageBitmap::SetPixel(const size_t x, const size_t y, const RGB& pixel) noexcept -> void
{
  m_buff.at((y * m_width) + x) = pixel;
}

#if __clang_major__ >= 16
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Wunsafe-buffer-usage"
#endif
void ImageBitmap::Load(const std::string& imageFilename)
{
  m_filename                                = imageFilename;
  const auto [rgbImage, width, height, bpp] = GetRGBImage();

  const auto* rgbPtr = rgbImage;
  Resize({static_cast<uint32_t>(width), static_cast<uint32_t>(height)});
  for (auto y = 0U; y < GetHeight(); ++y)
  {
    for (auto x = 0U; x < GetWidth(); ++x)
    {
      //NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic): stb_image requirement
      const auto blue = *rgbPtr;
      ++rgbPtr;
      const auto green = *rgbPtr;
      ++rgbPtr;
      const auto red = *rgbPtr;
      ++rgbPtr;
      const auto alpha = *rgbPtr;
      ++rgbPtr;
      //NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic): stb_image requirement

      if (0 == alpha)
      {
        SetPixel(x, y, RGB{});
      }
      else
      {
        SetPixel(x, y, RGB{/*.r = */ red, /*.g = */ green, /*.b = */ blue, /*.a = */ alpha});
      }
    }
  }

  ::stbi_image_free(rgbImage);
}
#if __clang_major__ >= 16
#pragma GCC diagnostic pop
#endif

auto ImageBitmap::GetRGBImage() const -> std::tuple<uint8_t*, int32_t, int32_t, int32_t>
{
  try
  {
    static constexpr auto DESIRED_CHANNELS = 4;

    auto width     = 0;
    auto height    = 0;
    auto bpp       = 0;
    auto* rgbImage = ::stbi_load(m_filename.c_str(), &width, &height, &bpp, DESIRED_CHANNELS);
    if (!rgbImage)
    {
      // NOLINTNEXTLINE(misc-include-cleaner): Waiting for C++20.
      throw std::runtime_error(std_fmt::format(R"(Could not load image file "{}".)", m_filename));
    }
    if ((0 == width) or (0 == height) or (0 == bpp))
    {
      throw std::runtime_error(
          // NOLINTNEXTLINE(misc-include-cleaner): Waiting for C++20.
          std_fmt::format("Error loading image \"{}\". width = {}, height = {}, bpp = {}.",
                          m_filename,
                          width,
                          height,
                          bpp));
    }

    return {rgbImage, width, height, bpp};
  }
  catch (const std::exception& e)
  {
    // NOLINTNEXTLINE(misc-include-cleaner): Waiting for C++20.
    throw std::runtime_error(std_fmt::format(
        R"(Could not load image file "{}". Exception: "{}".)", m_filename, e.what()));
  }
}

} // namespace GOOM::UTILS::GRAPHICS
