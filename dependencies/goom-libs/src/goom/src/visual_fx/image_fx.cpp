#include "image_fx.h"

#include "color/colormaps.h"
#include "color/colorutils.h"
#include "draw/goom_draw.h"
#include "goom_graphic.h"
#include "goom_plugin_info.h"
#include "logging_control.h"
#include "utils/graphics/image_bitmaps.h"
//#undef NO_LOGGING
#include "color/random_colormaps.h"
#include "goom/spimpl.h"
#include "logging.h"
#include "utils/goomrand.h"
#include "utils/mathutils.h"
#include "v2d.h"

#include <array>
#include <cmath>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#if __cplusplus <= 201402L
namespace GOOM
{
namespace VISUAL_FX
{
#else
namespace GOOM::VISUAL_FX
{
#endif

using COLOR::GammaCorrection;
using COLOR::GetBrighterColor;
using COLOR::GetColorMultiply;
using COLOR::GetLightenedColor;
using COLOR::IColorMap;
using COLOR::RandomColorMaps;
using DRAW::IGoomDraw;
using UTILS::GetRandInRange;
using UTILS::ImageBitmap;
using UTILS::Logging;
using UTILS::m_two_pi;

struct ImageChunk
{
  V2dInt startPos;
  std::array<Pixel, 4> pixels;
};
using ImageAsChunks = std::vector<ImageChunk>;

struct Image
{
  Image(std::shared_ptr<ImageBitmap> i,
        std::shared_ptr<const IColorMap> cm,
        const V2dInt& c,
        const int p,
        const int step)
    : image{std::move(i)},
      colorMap{std::move(cm)},
      startingCentre{c},
      pathCounter{p},
      counterStep{step}
  {
  }

  const std::shared_ptr<ImageBitmap> image;
  std::shared_ptr<const IColorMap> colorMap{};
  const V2dInt startingCentre;
  V2dInt centre{};
  int pathCounter{};
  int counterStep{};
  bool backwards{false};
};

class ImageFx::ImageFxImpl
{
public:
  explicit ImageFxImpl(const DRAW::IGoomDraw& draw,
                       const PluginInfo& goomInfo,
                       const std::string& resourcesDirectory) noexcept;

  void Start();
  void ApplyMultiple();

private:
  const IGoomDraw& m_draw;
  const PluginInfo& m_goomInfo;
  const std::string m_resourcesDirectory;
  uint64_t m_updateNum = 0;
  RandomColorMaps m_colorMaps{};
  std::vector<Image> m_images{};
  const V2dInt m_targetPos;
  auto GetNextStartingCentre(Image& image) -> V2dInt;
  static constexpr uint32_t MAX_PATH_COUNT = 500;
  std::vector<V2dInt> m_path;
  void InitPath();
  float m_tVal{0};
  bool m_tValBack = false;
  static constexpr float T_STEP = 0.01F;
  void ResetCentres();
  void UpdateCentres();
  static constexpr float GAMMA = 2.0F;
  static constexpr float GAMMA_BRIGHTNESS_THRESHOLD = 0.01F;
  GammaCorrection m_gammaCorrect{GAMMA, GAMMA_BRIGHTNESS_THRESHOLD};
  //auto GetGammaCorrection(float brightness, const Pixel& color) const -> Pixel;
};

ImageFx::ImageFx(const IGoomDraw& draw,
                 const PluginInfo& goomInfo,
                 const std::string& resourcesDirectory) noexcept
  : m_fxImpl{spimpl::make_unique_impl<ImageFxImpl>(draw, goomInfo, resourcesDirectory)}
{
}

void ImageFx::Start()
{
  m_fxImpl->Start();
}

void ImageFx::Finish()
{
  // nothing to do
}

auto ImageFx::GetFxName() const -> std::string
{
  return "image";
}

void ImageFx::ApplyMultiple()
{
  m_fxImpl->ApplyMultiple();
}

ImageFx::ImageFxImpl::ImageFxImpl(const IGoomDraw& draw,
                                  const PluginInfo& goomInfo,
                                  const std::string& resourcesDirectory) noexcept
  : m_draw{draw},
    m_goomInfo(goomInfo),
    m_resourcesDirectory{resourcesDirectory},
    m_targetPos{static_cast<int>(m_goomInfo.GetScreenInfo().width / 2),
                static_cast<int>(m_goomInfo.GetScreenInfo().height / 2)},
    m_path(MAX_PATH_COUNT + 1)
{
  InitPath();
}

void ImageFx::ImageFxImpl::Start()
{
  m_updateNum = 0;

  const auto image = std::make_shared<ImageBitmap>(m_resourcesDirectory + PATH_SEP + IMAGES_DIR +
                                                   PATH_SEP + "mountain_sunset100x65.png");
  m_images.emplace_back(
      Image{image, m_colorMaps.GetRandomColorMapPtr(), {200, 200}, MAX_PATH_COUNT / 4, 2});
  m_images.emplace_back(Image{image,
                              m_colorMaps.GetRandomColorMapPtr(),
                              {200, static_cast<int>(m_goomInfo.GetScreenInfo().height) - 200},
                              (2 * MAX_PATH_COUNT) / 4,
                              1});
  m_images.emplace_back(Image{image,
                              m_colorMaps.GetRandomColorMapPtr(),
                              {static_cast<int>(m_goomInfo.GetScreenInfo().width) - 200,
                               static_cast<int>(m_goomInfo.GetScreenInfo().height) - 200},
                              (3 * MAX_PATH_COUNT) / 4,
                              2});
  m_images.emplace_back(Image{image,
                              m_colorMaps.GetRandomColorMapPtr(),
                              {static_cast<int>(m_goomInfo.GetScreenInfo().width) - 200, 200},
                              (4 * MAX_PATH_COUNT) / 4,
                              3});
}

void ImageFx::ImageFxImpl::ApplyMultiple()
{
  ++m_updateNum;

  if (0 == (m_updateNum % 100))
  {
    for (auto& image : m_images)
    {
      image.colorMap = m_colorMaps.GetRandomColorMapPtr();
    }
  }

  constexpr float BRIGHTNESS = 0.2F;
  size_t imageIndex = 0;
  const auto getColor = [&]([[maybe_unused]] const int x, [[maybe_unused]] const int y,
                            const Pixel& b) {
    return GetBrighterColor(
        BRIGHTNESS, GetColorMultiply(b, m_images[imageIndex].colorMap->GetColor(m_tVal), false),
        //        GetColorBlend(b, m_colorMap->GetColor(m_tVal)),
        false);
  };
  const auto getLighterColor = [&]([[maybe_unused]] const int x, [[maybe_unused]] const int y,
                                   const Pixel& b) {
    return GetLightenedColor(getColor(x, y, b), 10);
  };


  const std::vector<IGoomDraw::GetBitmapColorFunc> getColors{getColor, getLighterColor};
  for (size_t i = 0; i < m_images.size(); ++i)
  {
    const ImageBitmap& bitmap = *m_images[i].image;
    imageIndex = i;
    m_draw.Bitmap(m_images[i].centre.x, m_images[i].centre.y, bitmap, getColors);
  }

  m_tVal += m_tValBack ? -T_STEP : +T_STEP;
  if (m_tVal > 1.0F)
  {
    m_tValBack = true;
    //    ResetCentres();
  }
  else if (m_tVal < 0.0F)
  {
    m_tValBack = false;
    //    ResetCentres();
  }
  UpdateCentres();
}

void ImageFx::ImageFxImpl::ResetCentres()
{
  m_images[0].centre = {200, 200};
  m_images[1].centre = {200, static_cast<int>(m_goomInfo.GetScreenInfo().height) - 200};
  m_images[2].centre = {static_cast<int>(m_goomInfo.GetScreenInfo().width) - 200,
                        static_cast<int>(m_goomInfo.GetScreenInfo().height) - 200};
  m_images[3].centre = {static_cast<int>(m_goomInfo.GetScreenInfo().width) - 200, 200};
}

void ImageFx::ImageFxImpl::UpdateCentres()
{
  for (auto& image : m_images)
  {
    const V2dInt start = GetNextStartingCentre(image);
    const auto x = static_cast<int>(
        stdnew::lerp(static_cast<float>(start.x), static_cast<float>(m_targetPos.x), m_tVal));
    const auto y = static_cast<int>(
        stdnew::lerp(static_cast<float>(start.y), static_cast<float>(m_targetPos.y), m_tVal));
    image.centre = {GetRandInRange(x - 20, x + 20), GetRandInRange(y - 20, y + 20)};
  }
}

auto ImageFx::ImageFxImpl::GetNextStartingCentre(Image& image) -> V2dInt
{
  const V2dInt coord = m_path[static_cast<size_t>(image.pathCounter)];
  image.pathCounter += image.backwards ? -image.counterStep : +image.counterStep;
  if (image.pathCounter <= 0)
  {
    image.pathCounter = 0;
    image.backwards = false;
  }
  if (static_cast<size_t>(image.pathCounter) >= m_path.size())
  {
    image.pathCounter = static_cast<int>(m_path.size() - 1);
    image.backwards = true;
  }
  return coord;
}

void ImageFx::ImageFxImpl::InitPath()
{
  const float angleStep = m_two_pi / static_cast<float>(m_path.size() - 1);
  const auto radius = static_cast<float>(m_goomInfo.GetScreenInfo().height / 2);
  float angle = 0.0;
  const auto x0 = static_cast<int32_t>(m_goomInfo.GetScreenInfo().width / 2);
  const auto y0 = static_cast<int32_t>(m_goomInfo.GetScreenInfo().height / 2);
  for (auto& coord : m_path)
  {
    coord = {x0 + static_cast<int>(radius * std::sin(angle)),
             y0 + static_cast<int>(radius * std::cos(angle))};
    angle += angleStep;
  }
}

#if __cplusplus <= 201402L
} // namespace VISUAL_FX
} // namespace GOOM
#else
} // namespace GOOM::VISUAL_FX
#endif
