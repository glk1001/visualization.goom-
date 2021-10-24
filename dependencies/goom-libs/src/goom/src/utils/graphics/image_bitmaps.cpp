#include "image_bitmaps.h"

#include "goom/goom_graphic.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../stb_image.h"

#include <format>
#include <stdexcept>

#if __cplusplus <= 201402L
namespace GOOM
{
namespace UTILS
{
#else
namespace GOOM::UTILS
{
#endif

void ImageBitmap::Resize(const size_t width, const size_t height)
{
  m_width = static_cast<uint32_t>(width);
  m_height = static_cast<uint32_t>(height);
  m_buff.resize(static_cast<size_t>(m_width) * static_cast<size_t>(m_height));
}

inline void ImageBitmap::SetPixel(const size_t x, const size_t y, const RGB& pixel)
{
  m_buff.at((y * m_width) + x) = pixel;
}

void ImageBitmap::Load(std::string imageFilename)
{
  m_filename = std::move(imageFilename);

  int width{};
  int height{};
  int bpp{};
  uint8_t* rgbImage{};
  try
  {
    rgbImage = stbi_load(m_filename.c_str(), &width, &height, &bpp, 4);
  }
  catch (std::exception& e)
  {
    throw std::runtime_error(
        std20::format(R"(Could not load image file "{}". Exception: "{}".)", m_filename, e.what()));
  }

  if (!rgbImage)
  {
    throw std::runtime_error(std20::format(R"(Could not load image file "{}".)", m_filename));
  }

  if (width == 0 || height == 0 || bpp == 0)
  {
    throw std::runtime_error(
        std20::format("Error loading image \"{}\". width = {}, height = {}, bpp = {}.", m_filename,
                      width, height, bpp));
  }

  const uint8_t* rgbPtr = rgbImage;
  Resize(static_cast<size_t>(width), static_cast<size_t>(height));
  for (size_t y = 0; y < GetHeight(); ++y)
  {
    for (size_t x = 0; x < GetWidth(); ++x)
    {
      uint8_t blue = *rgbPtr;
      rgbPtr++;
      uint8_t green = *rgbPtr;
      rgbPtr++;
      uint8_t red = *rgbPtr;
      rgbPtr++;
      const uint8_t alpha = *rgbPtr;
      rgbPtr++;

      if (alpha == 0)
      {
        red = 0;
        green = 0;
        blue = 0;
      }

      SetPixel(x, y, RGB{/*.r = */ red, /*.g = */ green, /*.b = */ blue, /*.a = */ alpha});
    }
  }

  stbi_image_free(rgbImage);
}

#if __cplusplus <= 201402L
} // namespace UTILS
} // namespace GOOM
#else
} // namespace GOOM::UTILS
#endif
