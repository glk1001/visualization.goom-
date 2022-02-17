#include "small_image_bitmaps.h"

//#undef NO_LOGGING

#include "goom/goom_config.h"
#include "goom/logging.h"
#include "utils/enumutils.h"
#include "utils/math/mathutils.h"

#include <array>
#include <format>
#include <string>
#include <utility>

namespace GOOM::UTILS::GRAPHICS
{

using MATH::IsEven;

const std::array<std::string, NUM<SmallImageBitmaps::ImageNames>> SmallImageBitmaps::IMAGE_NAMES{
    "circle", "sphere", "pink-flower", "red-flower", "orange-flower", "white-flower",
};

SmallImageBitmaps::SmallImageBitmaps(std::string resourcesDirectory)
  : m_resourcesDirectory{std::move(resourcesDirectory)}
{
  constexpr size_t BY_TWO = 2;
  for (size_t res = MIN_IMAGE_SIZE; res <= MAX_IMAGE_SIZE; res += BY_TWO)
  {
    for (size_t i = 0; i < NUM<ImageNames>; ++i)
    {
      const auto name = static_cast<ImageNames>(i);
      m_bitmapImages.emplace(GetImageKey(name, res), GetImageBitmapPtr(name, res));
      LogInfo("Loaded image bitmap: '{}'.", GetImageKey(name, res));
    }
  }
}

auto SmallImageBitmaps::GetImageBitmap(const ImageNames name, const size_t res) const
    -> const ImageBitmap&
{
  size_t imageRes = res;
  if (IsEven(imageRes))
  {
    imageRes = 2 == res ? (res + 1) : (res - 1);
  }
  return *m_bitmapImages.at(GetImageKey(name, imageRes));
}

auto SmallImageBitmaps::GetImageBitmapPtr(const ImageNames name, const size_t sizeOfImageSquare)
    -> std::unique_ptr<const ImageBitmap>
{
  return std::make_unique<const ImageBitmap>(GetImageFilename(name, sizeOfImageSquare));
}

inline auto SmallImageBitmaps::GetImageKey(const ImageNames name, const size_t sizeOfImageSquare)
    -> std::string
{
  return std20::format("{}_{:02}", IMAGE_NAMES.at(static_cast<size_t>(name)), sizeOfImageSquare);
}

auto SmallImageBitmaps::GetImageFilename(const ImageNames name,
                                         const size_t sizeOfImageSquare) const -> std::string
{
  const std::string imagesDir = m_resourcesDirectory + PATH_SEP + IMAGES_DIR;
  std::string filename =
      std20::format("{}/{}{:02}x{:02}.png", imagesDir, IMAGE_NAMES.at(static_cast<size_t>(name)),
                    sizeOfImageSquare, sizeOfImageSquare);
  return filename;
}

} // namespace GOOM::UTILS::GRAPHICS
