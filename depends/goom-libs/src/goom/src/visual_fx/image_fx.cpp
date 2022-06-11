#include "image_fx.h"

//#undef NO_LOGGING

#include "color/color_adjustment.h"
#include "color/colormaps.h"
#include "color/colorutils.h"
#include "color/random_colormaps.h"
#include "draw/goom_draw.h"
#include "fx_helper.h"
#include "goom/spimpl.h"
#include "goom_config.h"
#include "goom_graphic.h"
#include "goom_plugin_info.h"
#include "logging.h"
#include "point2d.h"
#include "utils/graphics/image_bitmaps.h"
#include "utils/math/goom_rand_base.h"
#include "utils/math/misc.h"
#include "utils/parallel_utils.h"
#include "utils/t_values.h"

#include <array>
#include <cmath>
#include <cstdint>
#include <memory>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

namespace GOOM::VISUAL_FX
{

using COLOR::ColorAdjustment;
using COLOR::GetAllSlimMaps;
using COLOR::GetBrighterColor;
using COLOR::IColorMap;
using COLOR::RandomColorMaps;
using DRAW::IGoomDraw;
using DRAW::MultiplePixels;
using UTILS::Logging;
using UTILS::Parallel;
using UTILS::TValue;
using UTILS::GRAPHICS::ImageBitmap;
using UTILS::MATH::HALF;
using UTILS::MATH::I_HALF;
using UTILS::MATH::IGoomRand;
using UTILS::MATH::Sq;
using UTILS::MATH::SqDistance;
using UTILS::MATH::TWO_PI;

static constexpr int32_t CHUNK_WIDTH = 4;
static constexpr int32_t CHUNK_HEIGHT = 4;
using ChunkPixels = std::array<std::array<Pixel, CHUNK_WIDTH>, CHUNK_HEIGHT>;

class ChunkedImage
{
public:
  ChunkedImage(std::shared_ptr<ImageBitmap> image, const PluginInfo& goomInfo) noexcept;

  struct ImageChunk
  {
    Point2dInt finalPosition;
    ChunkPixels pixels;
  };

  [[nodiscard]] auto GetNumChunks() const -> size_t;
  [[nodiscard]] auto GetImageChunk(size_t i) const -> const ImageChunk&;

  [[nodiscard]] auto GetStartPosition(size_t i) const -> const Point2dInt&;
  auto SetStartPosition(size_t i, const Point2dInt& pos) -> void;

private:
  using ImageAsChunks = std::vector<ImageChunk>;
  const std::shared_ptr<ImageBitmap> m_image;
  const PluginInfo& m_goomInfo;
  ImageAsChunks m_imageAsChunks;
  std::vector<Point2dInt> m_startPositions;
  [[nodiscard]] static auto SplitImageIntoChunks(const ImageBitmap& imageBitmap,
                                                 const PluginInfo& goomInfo) -> ImageAsChunks;
  static auto SetImageChunkPixels(const ImageBitmap& imageBitmap,
                                  int32_t yImage,
                                  int32_t xImage,
                                  ImageChunk& imageChunk) -> void;
};

class ImageFx::ImageFxImpl
{
public:
  ImageFxImpl(Parallel& parallel,
              const FxHelper& fxHelper,
              const std::string& resourcesDirectory) noexcept;

  [[nodiscard]] auto GetCurrentColorMapsNames() const noexcept -> std::vector<std::string>;
  auto SetWeightedColorMaps(const WeightedColorMaps& weightedColorMaps) noexcept -> void;

  auto Start() -> void;
  auto Resume() -> void;
  auto ApplyMultiple() -> void;

private:
  Parallel& m_parallel;
  IGoomDraw& m_draw;
  const PluginInfo& m_goomInfo;
  const IGoomRand& m_goomRand;
  const std::string m_resourcesDirectory;

  const int32_t m_availableWidth{
      static_cast<int32_t>(m_goomInfo.GetScreenInfo().width - CHUNK_WIDTH)};
  const int32_t m_availableHeight{
      static_cast<int32_t>(m_goomInfo.GetScreenInfo().height - CHUNK_HEIGHT)};
  const Point2dInt m_screenCentre{I_HALF * m_availableWidth, I_HALF* m_availableHeight};
  const float m_maxRadius{HALF * static_cast<float>(std::min(m_availableWidth, m_availableHeight))};
  [[nodiscard]] auto GetNewRandBrightnessFactor() const -> float;
  float m_randBrightnessFactor{GetNewRandBrightnessFactor()};

  std::shared_ptr<RandomColorMaps> m_colorMaps{GetAllSlimMaps(m_goomRand)};
  const IColorMap* m_currentColorMap{&GetRandomColorMap()};
  [[nodiscard]] auto GetRandomColorMap() const -> const IColorMap&;
  bool m_pixelColorIsDominant = false;
  static constexpr float DEFAULT_BRIGHTNESS_BASE = 0.1F;
  float m_brightnessBase = DEFAULT_BRIGHTNESS_BASE;

  std::vector<std::unique_ptr<ChunkedImage>> m_images{};
  ChunkedImage* m_currentImage{};
  static constexpr uint32_t NUM_STEPS = 400;
  static constexpr uint32_t T_DELAY_TIME = 15;
  TValue m_inOutT{TValue::StepType::CONTINUOUS_REPEATABLE, NUM_STEPS, {{1.0F, T_DELAY_TIME}}};
  float m_inOutTSq = 0.0F;
  Point2dInt m_floatingStartPosition{};
  TValue m_floatingT{TValue::StepType::CONTINUOUS_REVERSIBLE, NUM_STEPS, 1.0F};
  auto InitImage() -> void;

  auto DrawChunks() -> void;
  [[nodiscard]] auto GetPositionAdjustedBrightness(float brightness,
                                                   const Point2dInt& position) const -> float;
  auto DrawChunk(const Point2dInt& pos, float brightness, const ChunkPixels& pixels) const -> void;
  [[nodiscard]] auto GetNextChunkStartPosition(size_t i) const -> Point2dInt;
  [[nodiscard]] auto GetNextChunkPosition(const Point2dInt& nextStartPosition,
                                          const ChunkedImage::ImageChunk& imageChunk) const
      -> Point2dInt;
  [[nodiscard]] auto GetPixelColors(const Pixel& pixelColor, float brightness) const
      -> MultiplePixels;
  [[nodiscard]] auto GetMappedColor(const Pixel& pixelColor) const -> Pixel;

  auto UpdateImageStartPositions() -> void;
  auto ResetCurrentImage() -> void;
  auto ResetStartPositions() -> void;

  auto UpdateFloatingStartPositions() -> void;
  auto SetNewFloatingStartPosition() -> void;
  [[nodiscard]] auto GetChunkFloatingStartPosition(size_t i) const -> Point2dInt;

  static constexpr float GAMMA = 1.0F;
  const ColorAdjustment m_colorAdjust{GAMMA};
};

ImageFx::ImageFx(Parallel& parallel,
                 const FxHelper& fxHelper,
                 const std::string& resourcesDirectory) noexcept
  : m_pimpl{spimpl::make_unique_impl<ImageFxImpl>(parallel, fxHelper, resourcesDirectory)}
{
}

auto ImageFx::GetFxName() const noexcept -> std::string
{
  return "image";
}

auto ImageFx::GetCurrentColorMapsNames() const noexcept -> std::vector<std::string>
{
  return m_pimpl->GetCurrentColorMapsNames();
}

auto ImageFx::SetWeightedColorMaps(const WeightedColorMaps& weightedColorMaps) noexcept -> void
{
  m_pimpl->SetWeightedColorMaps(weightedColorMaps);
}

auto ImageFx::Start() noexcept -> void
{
  m_pimpl->Start();
}

auto ImageFx::Finish() noexcept -> void
{
  // nothing to do
}

auto ImageFx::Resume() noexcept -> void
{
  m_pimpl->Resume();
}

auto ImageFx::ApplyMultiple() noexcept -> void
{
  m_pimpl->ApplyMultiple();
}

ImageFx::ImageFxImpl::ImageFxImpl(Parallel& parallel,
                                  const FxHelper& fxHelper,
                                  const std::string& resourcesDirectory) noexcept
  : m_parallel{parallel},
    m_draw{fxHelper.GetDraw()},
    m_goomInfo{fxHelper.GetGoomInfo()},
    m_goomRand{fxHelper.GetGoomRand()},
    m_resourcesDirectory{resourcesDirectory}
{
}

inline auto ImageFx::ImageFxImpl::GetRandomColorMap() const -> const IColorMap&
{
  Expects(m_colorMaps != nullptr);
  return m_colorMaps->GetRandomColorMap(m_colorMaps->GetRandomGroup());
}

inline auto ImageFx::ImageFxImpl::GetCurrentColorMapsNames() const noexcept
    -> std::vector<std::string>
{
  return {m_colorMaps->GetColorMapsName()};
}

inline auto ImageFx::ImageFxImpl::SetWeightedColorMaps(
    const WeightedColorMaps& weightedColorMaps) noexcept -> void
{
  Expects(weightedColorMaps.mainColorMaps != nullptr);

  m_colorMaps = weightedColorMaps.mainColorMaps;
  m_pixelColorIsDominant = m_goomRand.ProbabilityOf(0.0F);
  m_randBrightnessFactor = GetNewRandBrightnessFactor();
}

inline auto ImageFx::ImageFxImpl::GetNewRandBrightnessFactor() const -> float
{
  static constexpr float MIN_FACTOR = 0.5F;
  static constexpr float MAX_FACTOR = 2.0F;
  const float maxRadiusSq = Sq(m_maxRadius);

  return 1.0F / m_goomRand.GetRandInRange(MIN_FACTOR * maxRadiusSq, MAX_FACTOR * maxRadiusSq);
}

inline auto ImageFx::ImageFxImpl::Resume() -> void
{
  static constexpr float MIN_BRIGHTNESS = 0.1F;
  m_brightnessBase = m_goomRand.GetRandInRange(MIN_BRIGHTNESS, 1.0F) * DEFAULT_BRIGHTNESS_BASE;
}

inline auto ImageFx::ImageFxImpl::Start() -> void
{

  InitImage();
  ResetCurrentImage();
  ResetStartPositions();
  SetNewFloatingStartPosition();
}

auto ImageFx::ImageFxImpl::InitImage() -> void
{
  // clang-format off
  static const std::array s_IMAGE_FILENAMES{
      "blossoms.jpg",
      "bokeh.jpg",
      "butterfly.jpg",
      "chameleon-tail.jpg",
      "galaxy.jpg",
      "mountain_sunset.png",
      "night-tree.jpg",
      "pattern1.jpg",
      "pattern2.jpg",
      "pattern3.jpg",
      "pattern4.jpg",
      "pattern5.jpg",
      "pretty-flowers.jpg",
  };
  // clang-format on
  std::array<size_t, s_IMAGE_FILENAMES.size()> randImageIndexes{};
  std::iota(randImageIndexes.begin(), randImageIndexes.end(), 0);
  m_goomRand.Shuffle(begin(randImageIndexes), end(randImageIndexes));

  const std::string imageDir = m_resourcesDirectory + PATH_SEP + IMAGES_DIR + PATH_SEP + "image_fx";
  static constexpr size_t MAX_IMAGES = 5;
  for (size_t i = 0; i < MAX_IMAGES; ++i)
  {
    const std::string imageFilename =
        imageDir + PATH_SEP + s_IMAGE_FILENAMES.at(randImageIndexes.at(i));
    m_images.emplace_back(
        std::make_unique<ChunkedImage>(std::make_shared<ImageBitmap>(imageFilename), m_goomInfo));
  }
}

inline auto ImageFx::ImageFxImpl::ResetCurrentImage() -> void
{
  m_currentImage =
      m_images[m_goomRand.GetRandInRange(0U, static_cast<uint32_t>(m_images.size()))].get();
}

auto ImageFx::ImageFxImpl::ResetStartPositions() -> void
{
  static constexpr float MIN_RADIUS_FRACTION = 0.7F;
  const float randMaxRadius = m_goomRand.GetRandInRange(MIN_RADIUS_FRACTION, 1.0F) * m_maxRadius;

  const size_t numChunks = m_currentImage->GetNumChunks();
  float radiusTheta = 0.0F;
  const float radiusThetaStep = TWO_PI / static_cast<float>(numChunks);

  for (size_t i = 0; i < numChunks; ++i)
  {
    static constexpr float SMALL_OFFSET = 0.4F;
    const float maxRadiusAdj =
        (1.0F - (SMALL_OFFSET * (1.0F + std::sin(radiusTheta)))) * randMaxRadius;
    const float radius = m_goomRand.GetRandInRange(10.0F, maxRadiusAdj);
    const float theta = m_goomRand.GetRandInRange(0.0F, TWO_PI);
    const Point2dInt startPos =
        m_screenCentre + Vec2dInt{static_cast<int32_t>((std::cos(theta) * radius)),
                                  static_cast<int32_t>((std::sin(theta) * radius))};
    m_currentImage->SetStartPosition(i, startPos);

    radiusTheta += radiusThetaStep;
  }
}

inline auto ImageFx::ImageFxImpl::GetChunkFloatingStartPosition(const size_t i) const -> Point2dInt
{
  static constexpr float MARGIN = 20.0F;
  static constexpr float MIN_RADIUS_FACTOR = 0.025F;
  static constexpr float MAX_RADIUS_FACTOR = 0.5F;
  const auto aRadius = (m_goomRand.GetRandInRange(MIN_RADIUS_FACTOR, MAX_RADIUS_FACTOR) *
                        static_cast<float>(m_availableWidth)) -
                       MARGIN;
  const auto bRadius = (m_goomRand.GetRandInRange(MIN_RADIUS_FACTOR, MAX_RADIUS_FACTOR) *
                        static_cast<float>(m_availableHeight)) -
                       MARGIN;
  const float theta =
      (TWO_PI * static_cast<float>(i)) / static_cast<float>(m_currentImage->GetNumChunks());
  const Point2dInt floatingStartPosition =
      m_screenCentre + Vec2dInt{static_cast<int32_t>((std::cos(theta) * aRadius)),
                                static_cast<int32_t>((std::sin(theta) * bRadius))};
  return floatingStartPosition;
}

inline auto ImageFx::ImageFxImpl::SetNewFloatingStartPosition() -> void
{
  m_floatingStartPosition =
      m_screenCentre - Vec2dInt{m_goomRand.GetRandInRange(CHUNK_WIDTH, m_availableWidth),
                                m_goomRand.GetRandInRange(CHUNK_HEIGHT, m_availableHeight)};
}

inline auto ImageFx::ImageFxImpl::ApplyMultiple() -> void
{
  DrawChunks();

  UpdateFloatingStartPositions();

  UpdateImageStartPositions();
}

inline auto ImageFx::ImageFxImpl::DrawChunks() -> void
{
  static constexpr float IN_OUT_FACTOR = 0.02F;
  const float brightness = m_brightnessBase + (IN_OUT_FACTOR * m_inOutT());

  const auto drawChunk = [this, &brightness](const size_t i)
  {
    const Point2dInt nextStartPosition = GetNextChunkStartPosition(i);
    const ChunkedImage::ImageChunk& imageChunk = m_currentImage->GetImageChunk(i);
    const Point2dInt nextChunkPosition = GetNextChunkPosition(nextStartPosition, imageChunk);
    const float adjustedBrightness = GetPositionAdjustedBrightness(brightness, nextChunkPosition);
    DrawChunk(nextChunkPosition, adjustedBrightness, imageChunk.pixels);
  };

  m_parallel.ForLoop(m_currentImage->GetNumChunks(), drawChunk);

  /*** 3 times slower
  for (size_t i = 0; i < m_currentImage->GetNumChunks(); ++i)
  {
    drawChunk(i);
  }
   **/
}

inline auto ImageFx::ImageFxImpl::GetPositionAdjustedBrightness(const float brightness,
                                                                const Point2dInt& position) const
    -> float
{
  return m_randBrightnessFactor *
         (brightness * static_cast<float>(SqDistance(position.x, position.y)));
}

inline auto ImageFx::ImageFxImpl::UpdateFloatingStartPositions() -> void
{
  m_floatingT.Increment();
  if (m_floatingT() <= 0.0F)
  {
    SetNewFloatingStartPosition();
  }
}

inline auto ImageFx::ImageFxImpl::UpdateImageStartPositions() -> void
{
  const bool delayJustFinishing = m_inOutT.DelayJustFinishing();

  m_inOutT.Increment();
  m_inOutTSq = Sq(m_inOutT());

  if (delayJustFinishing)
  {
    ResetCurrentImage();
    ResetStartPositions();
    SetNewFloatingStartPosition();
    m_floatingT.Reset(1.0F);
    m_currentColorMap = &GetRandomColorMap();
  }
}

inline auto ImageFx::ImageFxImpl::GetNextChunkStartPosition(const size_t i) const -> Point2dInt
{
  const Point2dInt startPos =
      lerp(m_currentImage->GetStartPosition(i),
           m_floatingStartPosition + Vec2dInt{GetChunkFloatingStartPosition(i)}, m_floatingT());
  return startPos;
}

inline auto ImageFx::ImageFxImpl::GetNextChunkPosition(
    const Point2dInt& nextStartPosition, const ChunkedImage::ImageChunk& imageChunk) const
    -> Point2dInt
{
  const Point2dInt nextChunkPosition =
      lerp(nextStartPosition, imageChunk.finalPosition, m_inOutT());
  return nextChunkPosition;
}

auto ImageFx::ImageFxImpl::DrawChunk(const Point2dInt& pos,
                                     const float brightness,
                                     const ChunkPixels& pixels) const -> void

{
  int32_t y = pos.y;
  for (size_t i = 0; i < CHUNK_HEIGHT; ++i)
  {
    const std::array<Pixel, CHUNK_WIDTH>& pixelRow = pixels[i];

    int32_t x = pos.x;
    for (const auto& xPixel : pixelRow)
    {
      if ((x < 0) || (x >= m_availableWidth))
      {
        continue;
      }
      if ((y < 0) || (y >= m_availableHeight))
      {
        continue;
      }
      const MultiplePixels pixelColors = GetPixelColors(xPixel, brightness);
      m_draw.DrawPixels({x, y}, pixelColors);

      ++x;
    }

    ++y;
  }
}

inline auto ImageFx::ImageFxImpl::GetPixelColors(const Pixel& pixelColor,
                                                 const float brightness) const -> MultiplePixels
{
  const Pixel mixedColor =
      IColorMap::GetColorMix(GetMappedColor(pixelColor), pixelColor, m_inOutTSq);
  const Pixel color0 = GetBrighterColor(brightness, mixedColor);
  const Pixel color1 = GetBrighterColor(0.5F * brightness, pixelColor);

  if (m_pixelColorIsDominant)
  {
    return {color1, color0};
  }

  return {color0, color1};
}

inline auto ImageFx::ImageFxImpl::GetMappedColor(const Pixel& pixelColor) const -> Pixel
{
  const float t = (pixelColor.RFlt() + pixelColor.GFlt() + pixelColor.BFlt()) / 3.0F;
  return m_currentColorMap->GetColor(t);
}

ChunkedImage::ChunkedImage(std::shared_ptr<ImageBitmap> image, const PluginInfo& goomInfo) noexcept
  : m_image{std::move(image)},
    m_goomInfo{goomInfo},
    m_imageAsChunks{SplitImageIntoChunks(*m_image, m_goomInfo)},
    m_startPositions(m_imageAsChunks.size())
{
}

inline auto ChunkedImage::GetNumChunks() const -> size_t
{
  return m_imageAsChunks.size();
}

inline auto ChunkedImage::GetImageChunk(const size_t i) const -> const ImageChunk&
{
  return m_imageAsChunks.at(i);
}

inline auto ChunkedImage::GetStartPosition(const size_t i) const -> const Point2dInt&
{
  return m_startPositions.at(i);
}

inline auto ChunkedImage::SetStartPosition(const size_t i, const Point2dInt& pos) -> void
{
  m_startPositions.at(i) = pos;
}

auto ChunkedImage::SplitImageIntoChunks(const ImageBitmap& imageBitmap, const PluginInfo& goomInfo)
    -> ImageAsChunks
{
  ImageAsChunks imageAsChunks{};

  const Point2dInt centre{goomInfo.GetScreenInfo().width / 2, goomInfo.GetScreenInfo().height / 2};
  const int32_t x0 = centre.x - static_cast<int32_t>(imageBitmap.GetWidth() / 2);
  const int32_t y0 = centre.y - static_cast<int32_t>(imageBitmap.GetHeight() / 2);

  assert(x0 >= 0);
  assert(y0 >= 0);

  const int32_t numYChunks = static_cast<int32_t>(imageBitmap.GetHeight()) / CHUNK_HEIGHT;
  const int32_t numXChunks = static_cast<int32_t>(imageBitmap.GetWidth()) / CHUNK_WIDTH;
  int32_t y = y0;
  int32_t yImage = 0;
  for (int32_t yChunk = 0; yChunk < numYChunks; ++yChunk)
  {
    int32_t x = x0;
    int32_t xImage = 0;
    for (int32_t xChunk = 0; xChunk < numXChunks; ++xChunk)
    {
      ImageChunk imageChunk;
      imageChunk.finalPosition = {x, y};
      SetImageChunkPixels(imageBitmap, yImage, xImage, imageChunk);
      imageAsChunks.emplace_back(imageChunk);

      x += CHUNK_WIDTH;
      xImage += CHUNK_WIDTH;
    }
    y += CHUNK_HEIGHT;
    yImage += CHUNK_HEIGHT;
  }

  return imageAsChunks;
}

auto ChunkedImage::SetImageChunkPixels(const ImageBitmap& imageBitmap,
                                       const int32_t yImage,
                                       const int32_t xImage,
                                       ChunkedImage::ImageChunk& imageChunk) -> void
{
  for (size_t yPixel = 0; yPixel < CHUNK_HEIGHT; ++yPixel)
  {
    const size_t yImageChunkPos = static_cast<size_t>(yImage) + yPixel;
    std::array<Pixel, CHUNK_WIDTH>& pixelRow = imageChunk.pixels.at(yPixel);
    for (size_t xPixel = 0; xPixel < CHUNK_WIDTH; ++xPixel)
    {
      pixelRow.at(xPixel) = imageBitmap(static_cast<size_t>(xImage) + xPixel, yImageChunkPos);
    }
  }
}

} // namespace GOOM::VISUAL_FX
