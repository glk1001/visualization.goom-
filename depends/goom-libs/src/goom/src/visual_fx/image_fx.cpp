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
#include "utils/parallel_utils.h"
#include "utils/t_values.h"
#include "v2d.h"

#include <array>
#undef NDEBUG
#include <cassert>
#include <cmath>
#include <cstdint>
#include <memory>
#include <numeric>
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
using COLOR::GetAllSlimMaps;
using COLOR::GetBrighterColor;
using COLOR::GetLightenedColor;
using COLOR::IColorMap;
using COLOR::RandomColorMaps;
using DRAW::IGoomDraw;
using UTILS::GetRandInRange;
using UTILS::ImageBitmap;
using UTILS::Logging;
using UTILS::m_two_pi;
using UTILS::Parallel;
using UTILS::ProbabilityOf;
using UTILS::Shuffle;
using UTILS::Sq;
using UTILS::SqDistance;
using UTILS::TValue;

constexpr int32_t CHUNK_WIDTH = 4;
constexpr int32_t CHUNK_HEIGHT = 4;
using ChunkPixels = std::array<std::array<Pixel, CHUNK_WIDTH>, CHUNK_HEIGHT>;

class ChunkedImage
{
public:
  ChunkedImage(std::shared_ptr<ImageBitmap> image, const PluginInfo& goomInfo) noexcept;

  struct ImageChunk
  {
    V2dInt finalPosition;
    ChunkPixels pixels;
  };

  [[nodiscard]] auto GetNumChunks() const -> size_t;
  [[nodiscard]] auto GetImageChunk(size_t i) const -> const ImageChunk&;

  [[nodiscard]] auto GetStartPosition(size_t i) const -> const V2dInt&;
  void SetStartPosition(size_t i, const V2dInt& pos);

private:
  using ImageAsChunks = std::vector<ImageChunk>;
  const std::shared_ptr<ImageBitmap> m_image;
  const PluginInfo& m_goomInfo;
  ImageAsChunks m_imageAsChunks;
  std::vector<V2dInt> m_startPositions;
  [[nodiscard]] static auto SplitImageIntoChunks(const ImageBitmap& imageBitmap,
                                                 const PluginInfo& goomInfo) -> ImageAsChunks;
  static void SetImageChunkPixels(const ImageBitmap& imageBitmap,
                                  int32_t yImage,
                                  int32_t xImage,
                                  ImageChunk& imageChunk);
};

class ImageFx::ImageFxImpl
{
public:
  ImageFxImpl(Parallel& parallel,
              DRAW::IGoomDraw& draw,
              const PluginInfo& goomInfo,
              const std::string& resourcesDirectory) noexcept;

  void SetWeightedColorMaps(std::shared_ptr<COLOR::RandomColorMaps> weightedMaps);

  void Start();
  void ApplyMultiple();

private:
  Parallel& m_parallel;
  IGoomDraw& m_draw;
  const PluginInfo& m_goomInfo;
  const std::string m_resourcesDirectory;
  const int32_t m_availableWidth;
  const int32_t m_availableHeight;
  const V2dInt m_screenCentre;
  const float m_maxRadius;
  const float m_maxDiameterSq;

  std::shared_ptr<RandomColorMaps> m_colorMaps;
  std::reference_wrapper<const IColorMap> m_currentColorMap;
  [[nodiscard]] auto GetRandomColorMap() const -> const IColorMap&;
  bool m_pixelColorIsDominant = false;

  std::vector<std::unique_ptr<ChunkedImage>> m_images{};
  ChunkedImage* m_currentImage{};
  static constexpr uint32_t NUM_STEPS = 400;
  static constexpr uint32_t T_DELAY_TIME = 15;
  TValue m_inOutT{TValue::StepType::CONTINUOUS_REPEATABLE, NUM_STEPS, {{1.0F, T_DELAY_TIME}}};
  float m_inOutTSq = 0.0F;
  V2dInt m_floatingStartPosition{};
  TValue m_floatingT{TValue::StepType::CONTINUOUS_REVERSIBLE, NUM_STEPS, 1.0F};
  void InitImage();

  void DrawChunks();
  void DrawChunk(const V2dInt& pos, float brightness, const ChunkPixels& pixels) const;
  [[nodiscard]] auto GetNextChunkStartPosition(size_t i) const -> V2dInt;
  [[nodiscard]] auto GetNextChunkPosition(const V2dInt& nextStartPosition,
                                          const ChunkedImage::ImageChunk& imageChunk) const
      -> V2dInt;
  [[nodiscard]] auto GetPixelColors(const Pixel& pixelColor, float brightness) const
      -> std::vector<Pixel>;
  [[nodiscard]] auto GetMappedColor(const Pixel& pixelColor) const -> Pixel;

  void UpdateImageStartPositions();
  void ResetCurrentImage();
  void ResetStartPositions();

  void UpdateFloatingStartPositions();
  void SetNewFloatingStartPosition();
  [[nodiscard]] auto GetChunkFloatingStartPosition(size_t i) const -> V2dInt;

  static constexpr float GAMMA = 1.0F / 1.0F;
  static constexpr float GAMMA_BRIGHTNESS_THRESHOLD = 0.01F;
  GammaCorrection m_gammaCorrect{GAMMA, GAMMA_BRIGHTNESS_THRESHOLD};
  //auto GetGammaCorrection(float brightness, const Pixel& color) const -> Pixel;
};

ImageFx::ImageFx(Parallel& parallel,
                 IGoomDraw& draw,
                 const PluginInfo& goomInfo,
                 const std::string& resourcesDirectory) noexcept
  : m_fxImpl{spimpl::make_unique_impl<ImageFxImpl>(parallel, draw, goomInfo, resourcesDirectory)}
{
}

void ImageFx::SetWeightedColorMaps(std::shared_ptr<COLOR::RandomColorMaps> weightedMaps)
{
  m_fxImpl->SetWeightedColorMaps(weightedMaps);
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

ImageFx::ImageFxImpl::ImageFxImpl(Parallel& parallel,
                                  IGoomDraw& draw,
                                  const PluginInfo& goomInfo,
                                  const std::string& resourcesDirectory) noexcept
  : m_parallel{parallel},
    m_draw{draw},
    m_goomInfo(goomInfo),
    m_resourcesDirectory{resourcesDirectory},
    m_availableWidth{static_cast<int32_t>(m_goomInfo.GetScreenInfo().width - CHUNK_WIDTH)},
    m_availableHeight{static_cast<int32_t>(m_goomInfo.GetScreenInfo().height - CHUNK_HEIGHT)},
    m_screenCentre{m_availableWidth / 2, m_availableHeight / 2},
    m_maxRadius{0.5F * static_cast<float>(std::min(m_availableWidth, m_availableHeight))},
    m_maxDiameterSq{2.0F * Sq(m_maxRadius)},
    m_colorMaps{GetAllSlimMaps()},
    m_currentColorMap{GetRandomColorMap()}
{
}

auto ImageFx::ImageFxImpl::GetRandomColorMap() const -> const IColorMap&
{
  assert(m_colorMaps);
  return m_colorMaps->GetRandomColorMap(m_colorMaps->GetRandomGroup());
}

void ImageFx::ImageFxImpl::SetWeightedColorMaps(std::shared_ptr<COLOR::RandomColorMaps> weightedMaps)
{
  m_colorMaps = weightedMaps;
  m_pixelColorIsDominant = ProbabilityOf(0.0F);
}

void ImageFx::ImageFxImpl::Start()
{

  InitImage();
  ResetCurrentImage();
  ResetStartPositions();
  SetNewFloatingStartPosition();
}

void ImageFx::ImageFxImpl::InitImage()
{
  using ImageFilenameArray = std::array<std::string, 13>;
  // clang-format off
  static const ImageFilenameArray s_imageFilenames{
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
  std::array<size_t, s_imageFilenames.size()> randImageIndexes{};
  std::iota(randImageIndexes.begin(), randImageIndexes.end(), 0);
  Shuffle(begin(randImageIndexes), end(randImageIndexes));

  const std::string imageDir = m_resourcesDirectory + PATH_SEP + IMAGES_DIR + PATH_SEP + "image_fx";
  constexpr size_t MAX_IMAGES = 5;
  for (size_t i = 0; i < MAX_IMAGES; ++i)
  {
    const std::string imageFilename =
        imageDir + PATH_SEP + s_imageFilenames.at(randImageIndexes.at(i));
    m_images.emplace_back(
        std::make_unique<ChunkedImage>(std::make_shared<ImageBitmap>(imageFilename), m_goomInfo));
  }
}

inline void ImageFx::ImageFxImpl::ResetCurrentImage()
{
  m_currentImage = m_images[GetRandInRange(0U, static_cast<uint32_t>(m_images.size()))].get();
}

inline void ImageFx::ImageFxImpl::ResetStartPositions()
{
  const auto randMaxRadius = GetRandInRange(0.7F, 1.0F) * m_maxRadius;

  float radiusTheta = 0.0F;
  const float radiusThetaStep = m_two_pi / static_cast<float>(m_currentImage->GetNumChunks());
  for (size_t i = 0; i < m_currentImage->GetNumChunks(); ++i)
  {
    constexpr float SMALL_OFFSET = 0.4F;
    const float maxRadiusAdj =
        (1.0F - SMALL_OFFSET * (1.0F + std::sin(radiusTheta))) * randMaxRadius;
    const float radius = GetRandInRange(10.0F, maxRadiusAdj);
    const float theta = GetRandInRange(0.0F, m_two_pi);
    const V2dInt startPos =
        m_screenCentre + V2dInt{static_cast<int32_t>((std::cos(theta) * radius)),
                                static_cast<int32_t>((std::sin(theta) * radius))};
    m_currentImage->SetStartPosition(i, startPos);

    radiusTheta += radiusThetaStep;
  }
}

inline auto ImageFx::ImageFxImpl::GetChunkFloatingStartPosition(const size_t i) const -> V2dInt
{
  constexpr float MARGIN = 20.0F;
  constexpr float MIN_RADIUS_FACTOR = 0.025F;
  constexpr float MAX_RADIUS_FACTOR = 0.5F;
  const auto aRadius =
      GetRandInRange(MIN_RADIUS_FACTOR, MAX_RADIUS_FACTOR) * static_cast<float>(m_availableWidth) -
      MARGIN;
  const auto bRadius =
      GetRandInRange(MIN_RADIUS_FACTOR, MAX_RADIUS_FACTOR) * static_cast<float>(m_availableHeight) -
      MARGIN;
  const float theta =
      m_two_pi * static_cast<float>(i) / static_cast<float>(m_currentImage->GetNumChunks());
  const V2dInt floatingStartPosition =
      m_screenCentre + V2dInt{static_cast<int32_t>((std::cos(theta) * aRadius)),
                              static_cast<int32_t>((std::sin(theta) * bRadius))};
  return floatingStartPosition;
}

inline void ImageFx::ImageFxImpl::SetNewFloatingStartPosition()
{
  m_floatingStartPosition =
      m_screenCentre - V2dInt{GetRandInRange(CHUNK_WIDTH, m_availableWidth),
                              GetRandInRange(CHUNK_HEIGHT, m_availableHeight)};
}

void ImageFx::ImageFxImpl::ApplyMultiple()
{
  DrawChunks();

  UpdateFloatingStartPositions();

  UpdateImageStartPositions();
}

inline void ImageFx::ImageFxImpl::DrawChunks()
{
  const float brightness = 0.02F + (0.03F * m_inOutT());

  const auto drawChunk = [&](const size_t i) {
    const V2dInt nextStartPosition = GetNextChunkStartPosition(i);
    const ChunkedImage::ImageChunk& imageChunk = m_currentImage->GetImageChunk(i);
    const V2dInt nextChunkPosition = GetNextChunkPosition(nextStartPosition, imageChunk);
    const float posAdjustedBrightness =
        brightness * (SqDistance(static_cast<float>(nextChunkPosition.x),
                                 static_cast<float>(nextChunkPosition.y)) /
                      m_maxDiameterSq);
    DrawChunk(nextChunkPosition, posAdjustedBrightness, imageChunk.pixels);
  };

  m_parallel.ForLoop(m_currentImage->GetNumChunks(), drawChunk);

  /*** 3 times slower
  for (size_t i = 0; i < m_currentImage->GetNumChunks(); ++i)
  {
    drawChunk(i);
  }
   **/
}

inline void ImageFx::ImageFxImpl::UpdateFloatingStartPositions()
{
  m_floatingT.Increment();
  if (m_floatingT() <= 0.0F)
  {
    SetNewFloatingStartPosition();
  }
}

inline void ImageFx::ImageFxImpl::UpdateImageStartPositions()
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
    m_currentColorMap = GetRandomColorMap();
  }
}

inline V2dInt ImageFx::ImageFxImpl::GetNextChunkStartPosition(const size_t i) const
{
  const V2dInt startPos =
      lerp(m_currentImage->GetStartPosition(i),
           m_floatingStartPosition + GetChunkFloatingStartPosition(i), m_floatingT());
  return startPos;
}

inline V2dInt ImageFx::ImageFxImpl::GetNextChunkPosition(
    const V2dInt& nextStartPosition, const ChunkedImage::ImageChunk& imageChunk) const
{
  const V2dInt nextChunkPosition = lerp(nextStartPosition, imageChunk.finalPosition, m_inOutT());
  return nextChunkPosition;
}

void ImageFx::ImageFxImpl::DrawChunk(const V2dInt& pos,
                                     const float brightness,
                                     const ChunkPixels& pixels) const

{
  int32_t y = pos.y;
  for (size_t yPixel = 0; yPixel < CHUNK_HEIGHT; ++yPixel)
  {
    int32_t x = pos.x;
    const std::array<Pixel, CHUNK_WIDTH>& pixelRow = pixels[yPixel];

    for (size_t xPixel = 0; xPixel < CHUNK_WIDTH; ++xPixel)
    {
      if (x < 0 || x > m_availableWidth)
      {
        continue;
      }
      if (y < 0 || y > m_availableHeight)
      {
        continue;
      }
      const std::vector<Pixel> pixelColors = GetPixelColors(pixelRow[xPixel], brightness);
      m_draw.DrawPixels(x, y, pixelColors, true);

      ++x;
    }

    ++y;
  }
}

inline auto ImageFx::ImageFxImpl::GetPixelColors(const Pixel& pixelColor,
                                                 const float brightness) const -> std::vector<Pixel>
{
  const Pixel mixedColor = IColorMap::GetColorMix(GetMappedColor(pixelColor), pixelColor, m_inOutTSq);
  const Pixel color0 = GetBrighterColor(brightness, mixedColor, false);
  const Pixel color1 = GetBrighterColor(0.5F * brightness, pixelColor, false);

  if (m_pixelColorIsDominant)
  {
    return {color1, color0};
  }

  return {color0, color1};
}

inline auto ImageFx::ImageFxImpl::GetMappedColor(const Pixel& pixelColor) const -> Pixel
{
  const float t = (pixelColor.RFlt() + pixelColor.GFlt() + pixelColor.BFlt()) / 3.0F;
  return m_currentColorMap.get().GetColor(t);
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

inline auto ChunkedImage::GetStartPosition(const size_t i) const -> const V2dInt&
{
  return m_startPositions.at(i);
}

inline void ChunkedImage::SetStartPosition(const size_t i, const V2dInt& pos)
{
  m_startPositions.at(i) = pos;
}

auto ChunkedImage::SplitImageIntoChunks(const ImageBitmap& imageBitmap, const PluginInfo& goomInfo)
    -> ImageAsChunks
{
  ImageAsChunks imageAsChunks{};

  const V2dInt centre{goomInfo.GetScreenInfo().width / 2, goomInfo.GetScreenInfo().height / 2};
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
      ImageChunk imageChunk{};
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

void ChunkedImage::SetImageChunkPixels(const ImageBitmap& imageBitmap,
                                       const int32_t yImage,
                                       const int32_t xImage,
                                       ChunkedImage::ImageChunk& imageChunk)
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

#if __cplusplus <= 201402L
} // namespace VISUAL_FX
} // namespace GOOM
#else
} // namespace GOOM::VISUAL_FX
#endif
