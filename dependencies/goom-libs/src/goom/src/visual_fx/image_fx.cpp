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

using COLOR::IColorMap;
using COLOR::GammaCorrection;
using COLOR::GetAllSlimMaps;
using COLOR::GetBrighterColor;
using COLOR::GetLightenedColor;
using COLOR::RandomColorMaps;
using DRAW::IGoomDraw;
using UTILS::GetRandInRange;
using UTILS::ImageBitmap;
using UTILS::Logging;
using UTILS::m_two_pi;
using UTILS::Parallel;
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

class ImageFx::ImageFxImpl
{
public:
  ImageFxImpl(Parallel& parallel,
              const DRAW::IGoomDraw& draw,
              const PluginInfo& goomInfo,
              const std::string& resourcesDirectory) noexcept;

  void SetWeightedColorMaps(std::shared_ptr<COLOR::RandomColorMaps> weightedMaps);

  void Start();
  void ApplyMultiple();

private:
  Parallel& m_parallel;
  const IGoomDraw& m_draw;
  const PluginInfo& m_goomInfo;
  const std::string m_resourcesDirectory;
  const int32_t m_availableWidth;
  const int32_t m_availableHeight;
  const float m_maxDiameterSq;
  uint64_t m_updateNum = 0;
  std::shared_ptr<RandomColorMaps> m_colorMaps;
  std::reference_wrapper<const IColorMap> m_currentColorMap;
  [[nodiscard]] auto GetRandomColorMap() const -> const IColorMap&;
  std::vector<std::unique_ptr<ChunkedImage>> m_images{};
  ChunkedImage* m_currentImage{};
  static constexpr uint32_t NUM_STEPS = 300;
  static constexpr uint32_t T_DELAY_TIME = 30;
  TValue m_inOutT{TValue::StepType::CONTINUOUS_REPEATABLE, NUM_STEPS, {{1.0F, T_DELAY_TIME}}};
  V2dInt m_floatingStartPosition{};
  TValue m_floatingT{TValue::StepType::CONTINUOUS_REVERSIBLE, NUM_STEPS};
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

  static constexpr float GAMMA = 2.0F;
  static constexpr float GAMMA_BRIGHTNESS_THRESHOLD = 0.01F;
  GammaCorrection m_gammaCorrect{GAMMA, GAMMA_BRIGHTNESS_THRESHOLD};
  //auto GetGammaCorrection(float brightness, const Pixel& color) const -> Pixel;
};

ImageFx::ImageFx(Parallel& parallel,
                 const IGoomDraw& draw,
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
                                  const IGoomDraw& draw,
                                  const PluginInfo& goomInfo,
                                  const std::string& resourcesDirectory) noexcept
  : m_parallel{parallel},
    m_draw{draw},
    m_goomInfo(goomInfo),
    m_resourcesDirectory{resourcesDirectory},
    m_availableWidth{static_cast<int32_t>(m_goomInfo.GetScreenInfo().width - CHUNK_WIDTH)},
    m_availableHeight{static_cast<int32_t>(m_goomInfo.GetScreenInfo().height - CHUNK_HEIGHT)},
    m_maxDiameterSq{2.0F * UTILS::Sq(static_cast<float>(std::min(m_availableWidth, m_availableHeight)))},
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
}

void ImageFx::ImageFxImpl::Start()
{
  m_updateNum = 0;

  InitImage();
  ResetCurrentImage();
  ResetStartPositions();
  SetNewFloatingStartPosition();
}

void ImageFx::ImageFxImpl::InitImage()
{
  static const std::array<std::string, 7> s_imageFilenames{
      "chameleon-tail.jpg",
      "mountain_sunset.png",
      "pattern1.jpg",
      "pattern2.jpg",
      "pattern3.jpg",
      "pattern4.jpg",
      "pattern5.jpg",
  };
  const std::string imageDir = m_resourcesDirectory + PATH_SEP + IMAGES_DIR + PATH_SEP + "image_fx";
  for (const auto& imageFilename : s_imageFilenames)
  {
    m_images.emplace_back(std::make_unique<ChunkedImage>(
        std::make_shared<ImageBitmap>(imageDir + PATH_SEP + imageFilename), m_goomInfo));
  }
}

inline void ImageFx::ImageFxImpl::ResetCurrentImage()
{
  m_currentImage = m_images[GetRandInRange(0U, static_cast<uint32_t>(m_images.size()))].get();
}

inline void ImageFx::ImageFxImpl::ResetStartPositions()
{
  const V2dInt centre{m_availableWidth / 2, m_availableHeight / 2};
  const auto maxRadius = 0.5F * static_cast<float>(std::min(m_availableWidth, m_availableHeight));

  for (size_t i = 0; i < m_currentImage->GetNumChunks(); ++i)
  {
    const float radius = GetRandInRange(10.0F, maxRadius);
    const float theta = GetRandInRange(0.0F, m_two_pi);
    const V2dInt startPos = centre + V2dInt{static_cast<int32_t>((std::cos(theta) * radius)),
                                            static_cast<int32_t>((std::sin(theta) * radius))};
    m_currentImage->SetStartPosition(i, startPos);
  }
}

inline void ImageFx::ImageFxImpl::SetNewFloatingStartPosition()
{
  m_floatingStartPosition = V2dInt{GetRandInRange(CHUNK_WIDTH, m_availableWidth),
                                   GetRandInRange(CHUNK_HEIGHT, m_availableHeight)};
}

void ImageFx::ImageFxImpl::ApplyMultiple()
{
  ++m_updateNum;

  DrawChunks();

  UpdateFloatingStartPositions();

  UpdateImageStartPositions();
}

inline void ImageFx::ImageFxImpl::DrawChunks()
{
  const float brightness = 0.07F + (0.07F * m_inOutT());

  const auto drawChunk = [&](const size_t i) {
    const V2dInt nextStartPosition = GetNextChunkStartPosition(i);
    const ChunkedImage::ImageChunk& imageChunk = m_currentImage->GetImageChunk(i);
    const V2dInt nextChunkPosition = GetNextChunkPosition(nextStartPosition, imageChunk);
    const float posAdjustedBrightness =
        brightness * (UTILS::SqDistance(static_cast<float>(nextChunkPosition.x),
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

  if (delayJustFinishing)
  {
    ResetCurrentImage();
    ResetStartPositions();
    SetNewFloatingStartPosition();
    m_floatingT.Reset();
    m_currentColorMap = GetRandomColorMap();
  }
}

inline V2dInt ImageFx::ImageFxImpl::GetNextChunkStartPosition(const size_t i) const
{
  const V2dInt startPos =
      lerp(m_currentImage->GetStartPosition(i), m_floatingStartPosition, m_floatingT());
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
      const std::vector<Pixel> pixelColors = GetPixelColors(pixelRow[xPixel], brightness);
      m_draw.DrawPixels(x, y, pixelColors, false);

      ++x;
    }

    ++y;
  }
}

inline auto ImageFx::ImageFxImpl::GetPixelColors(const Pixel& pixelColor,
                                                 const float brightness) const -> std::vector<Pixel>
{
  const Pixel mixedColor = IColorMap::GetColorMix(GetMappedColor(pixelColor), pixelColor, m_inOutT());
  //  const Pixel mixedColor = GetMappedColor(pixelColor);
  const Pixel color0 = GetBrighterColor(brightness, mixedColor, false);
  const Pixel color1 = GetBrighterColor(0.5F * brightness, pixelColor, false);
  return {color0, color1};
}

inline auto ImageFx::ImageFxImpl::GetMappedColor(const Pixel& pixelColor) const -> Pixel
{
  const float t = (pixelColor.RFlt() + pixelColor.GFlt() + pixelColor.BFlt()) / 3.0F;
  return m_currentColorMap.get().GetColor(t);
}

#if __cplusplus <= 201402L
} // namespace VISUAL_FX
} // namespace GOOM
#else
} // namespace GOOM::VISUAL_FX
#endif
