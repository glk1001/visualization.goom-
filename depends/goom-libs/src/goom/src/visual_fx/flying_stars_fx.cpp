#include "flying_stars_fx.h"

#include "color/colormaps.h"
#include "color/colorutils.h"
#include "draw/goom_draw.h"
#include "fx_helpers.h"
#include "goom/logging_control.h"
#include "goom_plugin_info.h"
//#undef NO_LOGGING
#include "color/random_colormaps.h"
#include "color/random_colormaps_manager.h"
#include "goom/logging.h"
#include "goom/spimpl.h"
#include "utils/goom_rand_base.h"
#include "utils/graphics/image_bitmaps.h"
#include "utils/graphics/small_image_bitmaps.h"
#include "utils/mathutils.h"
#include "v2d.h"

#undef NDEBUG
#include <cassert>
#include <cmath>
#include <cstddef>
#include <map>
#include <memory>
#include <stdexcept>
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
using COLOR::RandomColorMapsManager;
using COLOR::COLOR_DATA::ColorMapName;
using DRAW::IGoomDraw;
using UTILS::IGoomRand;
using UTILS::ImageBitmap;
using UTILS::m_pi;
using UTILS::m_third_pi;
using UTILS::m_two_pi;
using UTILS::SmallImageBitmaps;
using UTILS::Sq;
using UTILS::SqDistance;
using UTILS::Weights;

constexpr uint32_t MIN_STAR_AGE = 15;
constexpr uint32_t MAX_STAR_EXTRA_AGE = 50;

/* TODO:-- FAIRE PROPREMENT... BOAH... */

// The different modes of the visual FX.
enum class StarModes
{
  NO_FX = 0,
  FIREWORKS,
  RAIN,
  FOUNTAIN,
  _NUM // unused and must be last
};

struct Star
{
  V2dFlt pos{};
  V2dFlt velocity{};
  V2dFlt acceleration{};
  float age = 0.0;
  float vage = 0.0;
  std::shared_ptr<const IColorMap> dominantColormap{};
  std::shared_ptr<const IColorMap> dominantLowColormap{};
  std::shared_ptr<const IColorMap> currentColorMap{};
  std::shared_ptr<const IColorMap> currentLowColorMap{};
};

class FlyingStarsFx::FlyingStarsImpl
{
public:
  FlyingStarsImpl(const FxHelpers& fxHelpers, const SmallImageBitmaps& smallBitmaps) noexcept;

  void Start();

  void SetWeightedColorMaps(std::shared_ptr<RandomColorMaps> weightedMaps);
  void SetWeightedLowColorMaps(std::shared_ptr<RandomColorMaps> weightedMaps);

  void UpdateBuffers();

  void Finish();

private:
  IGoomDraw& m_draw;
  const PluginInfo& m_goomInfo;
  IGoomRand& m_goomRand;
  const int32_t m_halfWidth;
  const int32_t m_halfHeight;
  const float m_xMax;

  std::shared_ptr<RandomColorMaps> m_colorMaps{};
  std::shared_ptr<RandomColorMaps> m_lowColorMaps{};
  RandomColorMapsManager m_randomColorMapsManager{};
  uint32_t m_dominantColorMapID{};
  uint32_t m_dominantLowColorMapID{};
  uint32_t m_colorMapID{};
  uint32_t m_lowColorMapID{};
  bool m_megaColorMode = false;
  [[nodiscard]] auto GetNextColorMapName() const -> ColorMapName;
  [[nodiscard]] auto GetNextLowColorMapName() const -> ColorMapName;
  [[nodiscard]] auto GetNextAngleColorMapName() const -> ColorMapName;

  static constexpr float GAMMA = 1.0F / 1.0F;
  static constexpr float GAMMA_BRIGHTNESS_THRESHOLD = 0.1F;
  GammaCorrection m_gammaCorrect{GAMMA, GAMMA_BRIGHTNESS_THRESHOLD};
  [[nodiscard]] auto GetGammaCorrection(float brightness, const Pixel& color) const -> Pixel;

  static constexpr float MIN_SATURATION = 0.5F;
  static constexpr float MAX_SATURATION = 1.0F;
  static constexpr float MIN_LIGHTNESS = 0.5F;
  static constexpr float MAX_LIGHTNESS = 1.0F;

  ColorMode m_colorMode = ColorMode::MIX_COLORS;
  const Weights<ColorMode> m_colorModeWeights;
  void ChangeColorMode();
  [[nodiscard]] auto GetMixedColors(const Star& star, float t, float brightness)
      -> std::pair<Pixel, Pixel>;

  struct StarColorSet
  {
    Pixel color;
    Pixel lowColor;
    Pixel dominantColor;
    Pixel dominantLowColor;
  };
  [[nodiscard]] auto GetMixColors(const Star& star, float t) -> StarColorSet;
  [[nodiscard]] auto GetReversedMixColors(const Star& star, float t) -> StarColorSet;
  [[nodiscard]] auto GetSineMixColors(const Star& star) -> StarColorSet;
  [[nodiscard]] auto GetSimilarLowColors(const Star& star, float t) -> StarColorSet;
  [[nodiscard]] auto GetFinalMixedColors(const StarColorSet& starColorSet,
                                         float t,
                                         float brightness) -> std::pair<Pixel, Pixel>;

  uint32_t m_counter = 0;
  static constexpr uint32_t MAX_COUNT = 100;

  StarModes m_fxMode = StarModes::FIREWORKS;
  static constexpr uint32_t MAX_NUM_STARS = 1024;
  static constexpr uint32_t MIN_NUM_STARS = 100;
  uint32_t m_maxStars = MAX_NUM_STARS;
  std::vector<Star> m_stars{};
  static constexpr float OLD_AGE = 0.95F;
  uint32_t m_maxStarAge = 15;

  static constexpr float MIN_MIN_SIDE_WIND = -0.10F;
  static constexpr float MAX_MIN_SIDE_WIND = -0.01F;
  static constexpr float MIN_MAX_SIDE_WIND = +0.01F;
  static constexpr float MAX_MAX_SIDE_WIND = +0.10F;
  float m_minSideWind = 0.0F;
  float m_maxSideWind = 0.00001F;

  static constexpr float MIN_MIN_GRAVITY = +0.005F;
  static constexpr float MAX_MIN_GRAVITY = +0.010F;
  static constexpr float MIN_MAX_GRAVITY = +0.050F;
  static constexpr float MAX_MAX_GRAVITY = +0.090F;
  float m_minGravity = MAX_MIN_GRAVITY;
  float m_maxGravity = MAX_MAX_GRAVITY;

  // For fireworks largest bombs.
  float m_minAge = 1.0F - (99.0F / 100.0F);
  // For fireworks smallest bombs.
  float m_maxAge = 1.0F - (80.0F / 100.0F);

  bool m_useSingleBufferOnly = true;

  const Weights<StarModes> m_starModes;
  void CheckForStarEvents();
  void SoundEventOccurred();
  void UpdateWindAndGravity();
  void ChangeColorMaps();
  void UpdateStarColorMaps(float angle, Star& star);
  static constexpr size_t NUM_SEGMENTS = 20;
  std::array<ColorMapName, NUM_SEGMENTS> m_angleColorMapName{};
  void UpdateAngleColorMapNames();
  static auto GetSegmentNum(float angle) -> size_t;
  [[nodiscard]] auto GetDominantColorMapPtr(float angle) const -> std::shared_ptr<const IColorMap>;
  [[nodiscard]] auto GetDominantLowColorMapPtr(float angle) const
      -> std::shared_ptr<const IColorMap>;
  [[nodiscard]] auto GetCurrentColorMapPtr(float angle) const -> std::shared_ptr<const IColorMap>;
  [[nodiscard]] auto GetCurrentLowColorMapPtr(float angle) const
      -> std::shared_ptr<const IColorMap>;
  void DrawStars();
  static void UpdateStar(Star& star);
  [[nodiscard]] auto IsStarDead(const Star& star) const -> bool;
  enum class DrawMode
  {
    CIRCLES,
    LINES,
    DOTS,
    CIRCLES_AND_LINES,
  };
  DrawMode m_drawMode = DrawMode::CIRCLES;
  const Weights<DrawMode> m_drawModeWeights;
  void ChangeDrawMode();
  const SmallImageBitmaps& m_smallBitmaps;
  [[nodiscard]] auto GetImageBitmap(size_t size) const -> const ImageBitmap&;
  using DrawFunc = std::function<void(int32_t x1,
                                      int32_t y1,
                                      int32_t x2,
                                      int32_t y2,
                                      uint32_t size,
                                      const std::vector<Pixel>& colors)>;
  const std::map<DrawMode, const DrawFunc> m_drawFuncs{};
  [[nodiscard]] auto GetDrawFunc() const -> DrawFunc;
  void DrawStar(const Star& star, float flipSpeed, const DrawFunc& drawFunc);
  void DrawParticleCircle(int32_t x1,
                          int32_t y1,
                          int32_t x2,
                          int32_t y2,
                          uint32_t size,
                          const std::vector<Pixel>& colors);
  void DrawParticleLine(int32_t x1,
                        int32_t y1,
                        int32_t x2,
                        int32_t y2,
                        uint32_t size,
                        const std::vector<Pixel>& colors);
  void DrawParticleDot(int32_t x1,
                       int32_t y1,
                       int32_t x2,
                       int32_t y2,
                       uint32_t size,
                       const std::vector<Pixel>& colors);
  void RemoveDeadStars();

  struct StarModeParams
  {
    V2dInt pos{};
    float gravityFactor = 1.0F;
    float windFactor = 1.0F;
    float vage = 0.0;
    float radius = 1.0F;
  };
  [[nodiscard]] auto GetStarParams(float defaultRadius, float heightRatio) -> StarModeParams;
  [[nodiscard]] auto GetFireworksStarParams(float defaultRadius) const -> StarModeParams;
  [[nodiscard]] auto GetRainStarParams(float defaultRadius) const -> StarModeParams;
  [[nodiscard]] auto GetFountainStarParams(float defaultRadius) const -> StarModeParams;
  void AddStarBombs(const StarModeParams& starModeParams, size_t maxStarsInBomb);
  [[nodiscard]] auto GetMaxStarsInABomb(float heightRatio) const -> size_t;
  void AddABomb(const V2dInt& pos, float radius, float vage, float gravity, float sideWind);
  [[nodiscard]] auto GetBombAngle(const Star& star) const -> float;
  [[nodiscard]] auto GetRainBombAngle(const Star& star) const -> float;
  [[nodiscard]] auto GetFireworksBombAngle() const -> float;
  [[nodiscard]] auto GetFountainBombAngle(const Star& star) const -> float;

  static constexpr size_t MIN_DOT_SIZE = 3;
  static constexpr size_t MAX_DOT_SIZE = 5;
  static_assert(MAX_DOT_SIZE <= SmallImageBitmaps::MAX_IMAGE_SIZE, "Max dot size mismatch.");
};

FlyingStarsFx::FlyingStarsFx(const FxHelpers& fxHelpers,
                             const SmallImageBitmaps& smallBitmaps) noexcept
  : m_fxImpl{spimpl::make_unique_impl<FlyingStarsImpl>(fxHelpers, smallBitmaps)}
{
}

void FlyingStarsFx::SetWeightedColorMaps(const std::shared_ptr<RandomColorMaps> weightedMaps)
{
  m_fxImpl->SetWeightedColorMaps(weightedMaps);
}

void FlyingStarsFx::SetWeightedLowColorMaps(
    const std::shared_ptr<RandomColorMaps> weightedMaps)
{
  m_fxImpl->SetWeightedLowColorMaps(weightedMaps);
}

void FlyingStarsFx::Start()
{
  m_fxImpl->Start();
}

void FlyingStarsFx::Resume()
{
  // nothing to be done
}

void FlyingStarsFx::Suspend()
{
  // nothing to be done
}

void FlyingStarsFx::Finish()
{
  m_fxImpl->Finish();
}

auto FlyingStarsFx::GetFxName() const -> std::string
{
  return "Flying Stars FX";
}

void FlyingStarsFx::ApplyMultiple()
{
  m_fxImpl->UpdateBuffers();
}

FlyingStarsFx::FlyingStarsImpl::FlyingStarsImpl(const FxHelpers& fxHelpers,
                                                const SmallImageBitmaps& smallBitmaps) noexcept
  : m_draw{fxHelpers.GetDraw()},
    m_goomInfo{fxHelpers.GetGoomInfo()},
    m_goomRand{fxHelpers.GetGoomRand()},
    m_halfWidth{static_cast<int32_t>(m_goomInfo.GetScreenInfo().width / 2)},
    m_halfHeight{static_cast<int32_t>(m_goomInfo.GetScreenInfo().height / 2)},
    m_xMax{static_cast<float>(m_goomInfo.GetScreenInfo().width - 1)},
    // clang-format off
    m_colorModeWeights{
        m_goomRand,
        {
            { ColorMode::MIX_COLORS,         30 },
            { ColorMode::REVERSE_MIX_COLORS, 15 },
            { ColorMode::SIMILAR_LOW_COLORS, 10 },
            { ColorMode::SINE_MIX_COLORS,     5 },
        }
    },
    m_starModes{
        m_goomRand,
        {
            {StarModes::NO_FX, 11},
            {StarModes::FIREWORKS, 10},
            {StarModes::FOUNTAIN, 7},
            {StarModes::RAIN, 7},
        }
    },
    m_drawModeWeights{
        m_goomRand,
        {
            { DrawMode::DOTS,              30 },
            { DrawMode::CIRCLES,           20 },
            { DrawMode::LINES,             10 },
            { DrawMode::CIRCLES_AND_LINES, 15 },
        }
    },
    // clang-format on
    m_smallBitmaps{smallBitmaps},
    m_drawFuncs{
        {DrawMode::CIRCLES,
         [&](const int32_t x1,
             const int32_t y1,
             const int32_t x2,
             const int32_t y2,
             const uint32_t size,
             const std::vector<Pixel>& colors) {
           DrawParticleCircle(x1, y1, x2, y2, size, colors);
         }},
        {DrawMode::LINES,
         [&](const int32_t x1,
             const int32_t y1,
             const int32_t x2,
             const int32_t y2,
             const uint32_t size,
             const std::vector<Pixel>& colors) { DrawParticleLine(x1, y1, x2, y2, size, colors); }},
        {
            DrawMode::DOTS,
            [&](const int32_t x1,
                const int32_t y1,
                const int32_t x2,
                const int32_t y2,
                const uint32_t size,
                const std::vector<Pixel>& colors) {
              DrawParticleDot(x1, y1, x2, y2, size, colors);
            },
        }}
{
  m_stars.reserve(MAX_NUM_STARS);
}

inline auto FlyingStarsFx::FlyingStarsImpl::GetImageBitmap(const size_t size) const
    -> const ImageBitmap&
{
  return m_smallBitmaps.GetImageBitmap(SmallImageBitmaps::ImageNames::CIRCLE,
                                       stdnew::clamp(size, MIN_DOT_SIZE, MAX_DOT_SIZE));
}

inline void FlyingStarsFx::FlyingStarsImpl::SetWeightedColorMaps(
    const std::shared_ptr<RandomColorMaps> weightedMaps)
{
  m_colorMaps = weightedMaps;

  m_dominantColorMapID = m_randomColorMapsManager.AddColorMapInfo(
      {m_colorMaps, ColorMapName::_NULL, RandomColorMaps::ALL});
  m_colorMapID = m_randomColorMapsManager.AddColorMapInfo(
      {m_colorMaps, ColorMapName::_NULL, RandomColorMaps::ALL});

  m_colorMaps->SetSaturationLimits(MIN_SATURATION, MAX_SATURATION);
  m_colorMaps->SetLightnessLimits(MIN_LIGHTNESS, MAX_LIGHTNESS);
}

inline void FlyingStarsFx::FlyingStarsImpl::SetWeightedLowColorMaps(
    const std::shared_ptr<RandomColorMaps> weightedMaps)
{
  m_lowColorMaps = weightedMaps;

  m_dominantLowColorMapID = m_randomColorMapsManager.AddColorMapInfo(
      {m_lowColorMaps, ColorMapName::_NULL, RandomColorMaps::ALL});
  m_lowColorMapID = m_randomColorMapsManager.AddColorMapInfo(
      {m_lowColorMaps, ColorMapName::_NULL, RandomColorMaps::ALL});

  m_lowColorMaps->SetSaturationLimits(MIN_SATURATION, MAX_SATURATION);
  m_lowColorMaps->SetLightnessLimits(MIN_LIGHTNESS, MAX_LIGHTNESS);
}

inline void FlyingStarsFx::FlyingStarsImpl::Start()
{
  // nothing to do
}

void FlyingStarsFx::FlyingStarsImpl::Finish()
{
  // nothing to do
}

inline void FlyingStarsFx::FlyingStarsImpl::UpdateBuffers()
{
  ++m_counter;

  m_maxStars = m_goomRand.GetRandInRange(MIN_NUM_STARS, MAX_NUM_STARS);

  CheckForStarEvents();
  DrawStars();
  RemoveDeadStars();
}

void FlyingStarsFx::FlyingStarsImpl::CheckForStarEvents()
{
  if (m_stars.empty() || (m_goomInfo.GetSoundInfo().GetTimeSinceLastGoom() < 1))
  {
    SoundEventOccurred();
    if (m_goomRand.ProbabilityOfMInN(1, 20))
    {
      m_fxMode = m_starModes.GetRandomWeighted();
      ChangeColorMode();
      ChangeDrawMode();
    }
    else if (m_counter > MAX_COUNT)
    {
      m_counter = 0;
      ChangeColorMode();
      ChangeDrawMode();
    }
  }
  // m_fxMode = StarModes::rain;
}

void FlyingStarsFx::FlyingStarsImpl::DrawStars()
{
  const float flipSpeed = m_goomRand.GetRandInRange(0.1F, 10.0F);

  for (auto& star : m_stars)
  {
    UpdateStar(star);

    // Is it a dead particle?
    if (star.age >= static_cast<float>(m_maxStarAge))
    {
      continue;
    }

    DrawStar(star, flipSpeed, GetDrawFunc());
  }
}

inline void FlyingStarsFx::FlyingStarsImpl::ChangeDrawMode()
{
  m_drawMode = m_drawModeWeights.GetRandomWeighted();
}


inline auto FlyingStarsFx::FlyingStarsImpl::GetDrawFunc() const -> DrawFunc
{
  if (m_drawMode != DrawMode::CIRCLES_AND_LINES)
  {
    return m_drawFuncs.at(m_drawMode);
  }
  return m_goomRand.ProbabilityOfMInN(1, 2) ? m_drawFuncs.at(DrawMode::CIRCLES)
                                            : m_drawFuncs.at(DrawMode::LINES);
}

void FlyingStarsFx::FlyingStarsImpl::DrawStar(const Star& star,
                                              const float flipSpeed,
                                              const DrawFunc& drawFunc)
{
  const float tAge = star.age / static_cast<float>(m_maxStarAge);
  const float ageBrightness = 0.2F + ((0.8F * std::fabs(0.10F - tAge)) / 0.25F);
  const size_t numParts =
      tAge > OLD_AGE ? 4 : (2 + static_cast<size_t>(std::lround((1.0F - tAge) * 2.0F)));

  const auto x0 = static_cast<int32_t>(star.pos.x);
  const auto y0 = static_cast<int32_t>(star.pos.y);

  int32_t x1 = x0;
  int32_t y1 = y0;
  for (size_t j = 1; j <= numParts; ++j)
  {
    const int32_t x2 =
        x0 - static_cast<int32_t>(
                 0.5F * (1.0F + std::sin(flipSpeed * star.velocity.x * static_cast<float>(j))) *
                 star.velocity.x * static_cast<float>(j));
    const int32_t y2 =
        y0 - static_cast<int32_t>(
                 0.5F * (1.0F + std::cos(flipSpeed * star.velocity.y * static_cast<float>(j))) *
                 star.velocity.y * static_cast<float>(j));

    const float brightness =
        (2.7F * ageBrightness * static_cast<float>(j)) / static_cast<float>(numParts);
    const auto mixedColors = GetMixedColors(star, tAge, brightness);
    const std::vector<Pixel> colors = {mixedColors.first, mixedColors.second};
    const uint32_t size = tAge < OLD_AGE ? 1 : m_goomRand.GetRandInRange(2U, MAX_DOT_SIZE + 1);

    drawFunc(x1, y1, x2, y2, size, colors);

    x1 = x2;
    y1 = y2;
  }
}

void FlyingStarsFx::FlyingStarsImpl::DrawParticleCircle(const int32_t x1,
                                                        const int32_t y1,
                                                        [[maybe_unused]] const int32_t x2,
                                                        [[maybe_unused]] const int32_t y2,
                                                        const uint32_t size,
                                                        const std::vector<Pixel>& colors)
{
  if (m_useSingleBufferOnly)
  {
    m_draw.Circle(x1, y1, static_cast<int>(size), colors[0]);
  }
  else
  {
    m_draw.Circle(x1, y1, static_cast<int>(size), colors);
  }
}

void FlyingStarsFx::FlyingStarsImpl::DrawParticleLine(const int32_t x1,
                                                      const int32_t y1,
                                                      const int32_t x2,
                                                      const int32_t y2,
                                                      const uint32_t size,
                                                      const std::vector<Pixel>& colors)
{
  if (m_useSingleBufferOnly)
  {
    m_draw.Line(x1, y1, x2, y2, colors[0], static_cast<uint8_t>(size));
  }
  else
  {
    m_draw.Line(x1, y1, x2, y2, colors, static_cast<uint8_t>(size));
  }
}

void FlyingStarsFx::FlyingStarsImpl::DrawParticleDot(const int32_t x1,
                                                     const int32_t y1,
                                                     [[maybe_unused]] const int32_t x2,
                                                     [[maybe_unused]] const int32_t y2,
                                                     const uint32_t size,
                                                     const std::vector<Pixel>& colors)
{
  const auto getColor = [&]([[maybe_unused]] const size_t x, [[maybe_unused]] const size_t y,
                            const Pixel& b) { return GetColorMultiply(b, colors[0]); };
  const auto getLowColor = [&]([[maybe_unused]] const size_t x, [[maybe_unused]] const size_t y,
                               const Pixel& b) { return GetColorMultiply(b, colors[1]); };

  const ImageBitmap& bitmap = GetImageBitmap(size);

  if (m_useSingleBufferOnly)
  {
    m_draw.Bitmap(x1, y1, bitmap, getColor);
  }
  else
  {
    const std::vector<IGoomDraw::GetBitmapColorFunc> getColors{getColor, getLowColor};
    m_draw.Bitmap(x1, y1, bitmap, getColors);
  }
}

inline void FlyingStarsFx::FlyingStarsImpl::RemoveDeadStars()
{
  const auto isDead = [&](const Star& s) { return IsStarDead(s); };
#if __cplusplus <= 201703L
  m_stars.erase(std::remove_if(m_stars.begin(), m_stars.end(), isDead), m_stars.end());
#else
  const size_t numRemoved = std::erase_if(m_stars, isDead);
#endif
}

inline auto FlyingStarsFx::FlyingStarsImpl::IsStarDead(const Star& star) const -> bool
{
  constexpr int32_t DEAD_MARGIN = 64;

  if ((star.pos.x < -DEAD_MARGIN) ||
      (star.pos.x > static_cast<float>(m_goomInfo.GetScreenInfo().width + DEAD_MARGIN)))
  {
    return true;
  }
  if ((star.pos.y < -DEAD_MARGIN) ||
      (star.pos.y > static_cast<float>(m_goomInfo.GetScreenInfo().height + DEAD_MARGIN)))
  {
    return true;
  }

  return star.age >= static_cast<float>(this->m_maxStarAge);
}

inline void FlyingStarsFx::FlyingStarsImpl::ChangeColorMaps()
{
  m_randomColorMapsManager.ChangeColorMapNow(m_dominantColorMapID);
  m_randomColorMapsManager.ChangeColorMapNow(m_dominantLowColorMapID);

  m_megaColorMode = m_goomRand.ProbabilityOfMInN(1, 10);

  m_randomColorMapsManager.UpdateColorMapName(m_colorMapID, GetNextColorMapName());
  m_randomColorMapsManager.UpdateColorMapName(m_lowColorMapID, GetNextLowColorMapName());

  UpdateAngleColorMapNames();
}

inline auto FlyingStarsFx::FlyingStarsImpl::GetNextColorMapName() const -> ColorMapName
{
  return m_megaColorMode ? ColorMapName::_NULL
                         : m_colorMaps->GetRandomColorMapName(m_colorMaps->GetRandomGroup());
}

inline auto FlyingStarsFx::FlyingStarsImpl::GetNextLowColorMapName() const -> ColorMapName
{
  return m_megaColorMode ? ColorMapName::_NULL
                         : m_lowColorMaps->GetRandomColorMapName(m_lowColorMaps->GetRandomGroup());
}

inline auto FlyingStarsFx::FlyingStarsImpl::GetNextAngleColorMapName() const -> ColorMapName
{
  return m_megaColorMode ? m_colorMaps->GetRandomColorMapName()
                         : m_colorMaps->GetRandomColorMapName(m_colorMaps->GetRandomGroup());
}

void FlyingStarsFx::FlyingStarsImpl::UpdateStarColorMaps(const float angle, Star& star)
{
  // TODO Get colormap based on current mode.
  if (m_goomRand.ProbabilityOfMInN(10, 20))
  {
    star.dominantColormap = m_randomColorMapsManager.GetColorMapPtr(m_dominantColorMapID);
    star.dominantLowColormap = m_randomColorMapsManager.GetColorMapPtr(m_dominantLowColorMapID);
    star.currentColorMap = m_randomColorMapsManager.GetColorMapPtr(m_colorMapID);
    star.currentLowColorMap = m_randomColorMapsManager.GetColorMapPtr(m_lowColorMapID);
  }
  else
  {
    star.dominantColormap = GetDominantColorMapPtr(angle);
    star.dominantLowColormap = GetDominantLowColorMapPtr(angle);
    star.currentColorMap = GetCurrentColorMapPtr(angle);
    star.currentLowColorMap = GetCurrentLowColorMapPtr(angle);
  }

  assert(star.dominantColormap);
  assert(star.dominantLowColormap);
  assert(star.currentColorMap);
  assert(star.currentLowColorMap);
}

void FlyingStarsFx::FlyingStarsImpl::UpdateAngleColorMapNames()
{
  for (size_t i = 0; i < NUM_SEGMENTS; ++i)
  {
    m_angleColorMapName.at(i) = GetNextAngleColorMapName();
  }
}

auto FlyingStarsFx::FlyingStarsImpl::GetDominantColorMapPtr(const float angle) const
    -> std::shared_ptr<const IColorMap>
{
  return std::const_pointer_cast<const IColorMap>(
      m_colorMaps->GetColorMapPtr(m_angleColorMapName.at(GetSegmentNum(angle))));
}

auto FlyingStarsFx::FlyingStarsImpl::GetDominantLowColorMapPtr(const float angle) const
    -> std::shared_ptr<const IColorMap>
{
  return std::const_pointer_cast<const IColorMap>(
      m_colorMaps->GetColorMapPtr(m_angleColorMapName.at(GetSegmentNum(angle))));
}

auto FlyingStarsFx::FlyingStarsImpl::GetCurrentColorMapPtr(const float angle) const
    -> std::shared_ptr<const IColorMap>
{
  return std::const_pointer_cast<const IColorMap>(
      m_colorMaps->GetColorMapPtr(m_angleColorMapName.at(GetSegmentNum(angle))));
}

auto FlyingStarsFx::FlyingStarsImpl::GetCurrentLowColorMapPtr(const float angle) const
    -> std::shared_ptr<const IColorMap>
{
  return std::const_pointer_cast<const IColorMap>(
      m_colorMaps->GetColorMapPtr(m_angleColorMapName.at(GetSegmentNum(angle))));
}

auto FlyingStarsFx::FlyingStarsImpl::GetSegmentNum(const float angle) -> size_t
{
  const float segmentSize = m_two_pi / static_cast<float>(NUM_SEGMENTS);
  float a = segmentSize;
  for (size_t i = 0; i < NUM_SEGMENTS; ++i)
  {
    if (angle <= a)
    {
      return i;
    }
    a += segmentSize;
  }
  throw std::logic_error("Angle too large.");
}

void FlyingStarsFx::FlyingStarsImpl::ChangeColorMode()
{
  m_colorMode = m_colorModeWeights.GetRandomWeighted();
}

auto FlyingStarsFx::FlyingStarsImpl::GetMixedColors(const Star& star,
                                                    const float t,
                                                    const float brightness)
    -> std::pair<Pixel, Pixel>
{
  StarColorSet starColorSet;

  switch (m_colorMode)
  {
    case ColorMode::SINE_MIX_COLORS:
      starColorSet = GetSineMixColors(star);
      break;
    case ColorMode::MIX_COLORS:
      starColorSet = GetMixColors(star, t);
      break;
    case ColorMode::SIMILAR_LOW_COLORS:
      starColorSet = GetSimilarLowColors(star, t);
      break;
    case ColorMode::REVERSE_MIX_COLORS:
      starColorSet = GetReversedMixColors(star, t);
      break;
    default:
      throw std::logic_error("Unknown ColorMode enum.");
  }

  return GetFinalMixedColors(starColorSet, t, brightness);
}

inline auto FlyingStarsFx::FlyingStarsImpl::GetMixColors(const Star& star, const float t)
    -> StarColorSet
{
  return {star.currentColorMap->GetColor(t), star.currentLowColorMap->GetColor(t),
          star.dominantColormap->GetColor(t), star.dominantLowColormap->GetColor(t)};
}

inline auto FlyingStarsFx::FlyingStarsImpl::GetReversedMixColors(const Star& star, const float t)
    -> StarColorSet
{
  return GetMixColors(star, 1.0F - t);
}

inline auto FlyingStarsFx::FlyingStarsImpl::GetSimilarLowColors(const Star& star, const float t)
    -> StarColorSet
{
  StarColorSet starColorSet = GetMixColors(star, t);
  starColorSet.dominantLowColor = starColorSet.dominantColor;
  return starColorSet;
}

inline auto FlyingStarsFx::FlyingStarsImpl::GetSineMixColors(const Star& star) -> StarColorSet
{
  constexpr float FREQ = 20.0F;
  constexpr float T_MIX_FACTOR = 0.5F;
  constexpr float Z_STEP = 0.1F;
  static float s_z = 0.0F;

  const float tSin = T_MIX_FACTOR * (1.0F + std::sin(FREQ * s_z));

  StarColorSet starColorSet;
  starColorSet.color = star.currentColorMap->GetColor(tSin);
  starColorSet.lowColor = star.currentLowColorMap->GetColor(tSin);
  starColorSet.dominantColor = star.dominantColormap->GetColor(tSin);
  starColorSet.dominantLowColor = star.dominantLowColormap->GetColor(tSin);

  s_z += Z_STEP;

  return starColorSet;
}

inline auto FlyingStarsFx::FlyingStarsImpl::GetFinalMixedColors(const StarColorSet& starColorSet,
                                                                const float t,
                                                                const float brightness)
    -> std::pair<Pixel, Pixel>
{
  constexpr float MIN_MIX = 0.2F;
  constexpr float MAX_MIX = 0.8F;
  const float tMix = stdnew::lerp(MIN_MIX, MAX_MIX, t);
  const Pixel mixedColor = GetGammaCorrection(
      brightness, IColorMap::GetColorMix(starColorSet.color, starColorSet.dominantColor, tMix));
  const Pixel mixedLowColor = GetLightenedColor(
      IColorMap::GetColorMix(starColorSet.lowColor, starColorSet.dominantLowColor, tMix), 10.0F);
  const Pixel remixedLowColor =
      m_colorMode == ColorMode::SIMILAR_LOW_COLORS
          ? mixedLowColor
          : GetGammaCorrection(brightness, IColorMap::GetColorMix(mixedColor, mixedLowColor, 0.4F));

  return {mixedColor, remixedLowColor};
}

inline auto FlyingStarsFx::FlyingStarsImpl::GetGammaCorrection(const float brightness,
                                                               const Pixel& color) const -> Pixel
{
  // if constexpr (GAMMA == 1.0F)
  if (1.0F == GAMMA)
  {
    return GetBrighterColor(brightness, color);
  }
  return m_gammaCorrect.GetCorrection(brightness, color);
}

/**
 * Met a jour la position et vitesse d'une particule.
 */
inline void FlyingStarsFx::FlyingStarsImpl::UpdateStar(Star& star)
{
  star.pos += star.velocity;
  star.velocity += star.acceleration;
  star.age += star.vage;
}

/**
 * Ajoute de nouvelles particules au moment d'un evenement sonore.
 */
void FlyingStarsFx::FlyingStarsImpl::SoundEventOccurred()
{
  if (m_fxMode == StarModes::NO_FX)
  {
    return;
  }

  m_maxStarAge = MIN_STAR_AGE + m_goomRand.GetNRand(MAX_STAR_EXTRA_AGE);
  m_useSingleBufferOnly = m_goomRand.ProbabilityOfMInN(1, 100);

  UpdateWindAndGravity();
  ChangeColorMaps();

  // Why 200 ? Because the FX was developed on 320x200.
  constexpr float WIDTH = 320.0F;
  constexpr float HEIGHT = 200.0F;
  constexpr float MIN_HEIGHT = 50.0F;
  const auto heightRatio = static_cast<float>(m_goomInfo.GetScreenInfo().height) / HEIGHT;
  const float defaultRadius = (1.0F + m_goomInfo.GetSoundInfo().GetGoomPower()) *
                              (m_goomRand.GetRandInRange(MIN_HEIGHT, HEIGHT) / WIDTH);

  const StarModeParams starParams = GetStarParams(defaultRadius, heightRatio);
  const size_t maxStarsInBomb = GetMaxStarsInABomb(heightRatio);

  AddStarBombs(starParams, maxStarsInBomb);
}

inline void FlyingStarsFx::FlyingStarsImpl::UpdateWindAndGravity()
{
  if (m_goomRand.ProbabilityOfMInN(1, 10))
  {
    m_minSideWind = m_goomRand.GetRandInRange(MIN_MIN_SIDE_WIND, MAX_MIN_SIDE_WIND);
    m_maxSideWind = m_goomRand.GetRandInRange(MIN_MAX_SIDE_WIND, MAX_MAX_SIDE_WIND);
    m_minGravity = m_goomRand.GetRandInRange(MIN_MIN_GRAVITY, MAX_MIN_GRAVITY);
    m_maxGravity = m_goomRand.GetRandInRange(MIN_MAX_GRAVITY, MAX_MAX_GRAVITY);
  }
}

inline auto FlyingStarsFx::FlyingStarsImpl::GetMaxStarsInABomb(const float heightRatio) const
    -> size_t
{
  const auto maxStarsInBomb = static_cast<size_t>(
      heightRatio * (100.0F + ((m_goomInfo.GetSoundInfo().GetGoomPower() + 1.0F) *
                               m_goomRand.GetRandInRange(0.0F, 150.0F))));

  if (m_goomInfo.GetSoundInfo().GetTimeSinceLastBigGoom() < 1)
  {
    return 2 * maxStarsInBomb;
  }

  return maxStarsInBomb;
}

auto FlyingStarsFx::FlyingStarsImpl::GetStarParams(const float defaultRadius,
                                                   const float heightRatio) -> StarModeParams
{
  StarModeParams starParams;

  switch (m_fxMode)
  {
    case StarModes::FIREWORKS:
      starParams = GetFireworksStarParams(defaultRadius);
      break;
    case StarModes::RAIN:
      starParams = GetRainStarParams(defaultRadius);
      break;
    case StarModes::FOUNTAIN:
      m_maxStarAge = static_cast<uint32_t>(static_cast<float>(m_maxStarAge) * (2.0F / 3.0F));
      starParams = GetFountainStarParams(defaultRadius);
      break;
    default:
      throw std::logic_error("Unknown StarModes enum.");
  }

  starParams.radius *= heightRatio;
  if (m_goomInfo.GetSoundInfo().GetTimeSinceLastBigGoom() < 1)
  {
    starParams.radius *= 1.5F;
  }

  return starParams;
}

auto FlyingStarsFx::FlyingStarsImpl::GetFireworksStarParams(const float defaultRadius) const
    -> StarModeParams
{
  StarModeParams starParams;

  const auto rsq = static_cast<float>(m_halfHeight * m_halfHeight);
  while (true)
  {
    starParams.pos.x = static_cast<int32_t>(m_goomRand.GetNRand(m_goomInfo.GetScreenInfo().width));
    starParams.pos.y = static_cast<int32_t>(m_goomRand.GetNRand(m_goomInfo.GetScreenInfo().height));
    const float sqDist = SqDistance(static_cast<float>(starParams.pos.x - m_halfWidth),
                                    static_cast<float>(starParams.pos.y - m_halfHeight));
    if (sqDist < rsq)
    {
      break;
    }
  }

  constexpr float RADIUS_FACTOR = 1.0F;
  constexpr float INITIAL_WIND_FACTOR = 0.1F;
  constexpr float INITIAL_GRAVITY_FACTOR = 0.4F;
  starParams.radius = RADIUS_FACTOR * defaultRadius;
  starParams.vage = m_maxAge * (1.0F - m_goomInfo.GetSoundInfo().GetGoomPower());
  starParams.windFactor = INITIAL_WIND_FACTOR;
  starParams.gravityFactor = INITIAL_GRAVITY_FACTOR;

  return starParams;
}

auto FlyingStarsFx::FlyingStarsImpl::GetRainStarParams(const float defaultRadius) const
    -> StarModeParams
{
  StarModeParams starParams;

  const auto x0 = static_cast<int32_t>(m_goomInfo.GetScreenInfo().width / 25);
  starParams.pos.x =
      m_goomRand.GetRandInRange(x0, static_cast<int32_t>(m_goomInfo.GetScreenInfo().width) - x0);
  starParams.pos.y = -m_goomRand.GetRandInRange(3, 64);

  constexpr float RADIUS_FACTOR = 1.5F;
  constexpr float INITIAL_VAGE = 0.002F;
  constexpr float INITIAL_WIND_FACTOR = 1.0F;
  constexpr float INITIAL_GRAVITY_FACTOR = 0.4F;
  starParams.radius = RADIUS_FACTOR * defaultRadius;
  starParams.vage = INITIAL_VAGE;
  starParams.windFactor = INITIAL_WIND_FACTOR;
  starParams.gravityFactor = INITIAL_GRAVITY_FACTOR;

  return starParams;
}

auto FlyingStarsFx::FlyingStarsImpl::GetFountainStarParams(const float defaultRadius) const
    -> StarModeParams
{
  StarModeParams starParams;

  const int32_t x0 = m_halfWidth / 5;
  starParams.pos.x = m_goomRand.GetRandInRange(m_halfWidth - x0, m_halfWidth + x0);
  starParams.pos.y =
      static_cast<int32_t>(m_goomInfo.GetScreenInfo().height + m_goomRand.GetRandInRange(3U, 64U));

  constexpr float INITIAL_VAGE = 0.001F;
  constexpr float INITIAL_WIND_FACTOR = 1.0F;
  constexpr float INITIAL_GRAVITY_FACTOR = 1.0F;
  starParams.radius = 1.0F + defaultRadius;
  starParams.vage = INITIAL_VAGE;
  starParams.windFactor = INITIAL_WIND_FACTOR;
  starParams.gravityFactor = INITIAL_GRAVITY_FACTOR;

  return starParams;
}

void FlyingStarsFx::FlyingStarsImpl::AddStarBombs(const StarModeParams& starModeParams,
                                                  const size_t maxStarsInBomb)
{
  const float sideWind =
      starModeParams.windFactor * m_goomRand.GetRandInRange(m_minSideWind, m_maxSideWind);
  const float gravity =
      starModeParams.gravityFactor * m_goomRand.GetRandInRange(m_minGravity, m_maxGravity);

  for (size_t i = 0; i < maxStarsInBomb; ++i)
  {
    m_randomColorMapsManager.ChangeColorMapNow(m_colorMapID);
    m_randomColorMapsManager.ChangeColorMapNow(m_lowColorMapID);
    AddABomb(starModeParams.pos, starModeParams.radius, starModeParams.vage, gravity, sideWind);
  }
}

/**
 * Cree une nouvelle 'bombe', c'est a dire une particule appartenant a une fusee d'artifice.
 */
void FlyingStarsFx::FlyingStarsImpl::AddABomb(const V2dInt& pos,
                                              const float radius,
                                              const float vage,
                                              const float gravity,
                                              const float sideWind)
{
  if (m_stars.size() >= m_maxStars)
  {
    return;
  }

  m_stars.emplace_back();
  Star& star = m_stars[m_stars.size() - 1];

  star.pos = pos.ToFlt();

  const float bombRadius = radius * m_goomRand.GetRandInRange(0.01F, 2.0F);
  const float bombAngle = GetBombAngle(star);

  constexpr float RADIUS_OFFSET = -0.2F;
  star.velocity.x = bombRadius * std::cos(bombAngle);
  star.velocity.y = RADIUS_OFFSET + (bombRadius * std::sin(bombAngle));

  star.acceleration.x = sideWind;
  star.acceleration.y = gravity;

  star.age = m_goomRand.GetRandInRange(m_minAge, 0.5F * m_maxAge);
  star.vage = std::max(m_minAge, vage);

  UpdateStarColorMaps(bombAngle, star);
}

inline auto FlyingStarsFx::FlyingStarsImpl::GetBombAngle(const Star& star) const -> float
{
  switch (m_fxMode)
  {
    case StarModes::FIREWORKS:
      return GetFireworksBombAngle();
    case StarModes::RAIN:
      return GetRainBombAngle(star);
    case StarModes::FOUNTAIN:
      return GetFountainBombAngle(star);
    default:
      throw std::logic_error("Unknown StarModes enum.");
  }
}

inline auto FlyingStarsFx::FlyingStarsImpl::GetFireworksBombAngle() const -> float
{
  constexpr float MIN_FIREWORKS_ANGLE = 0.0F;
  constexpr float MAX_FIREWORKS_ANGLE = m_two_pi;

  return m_goomRand.GetRandInRange(MIN_FIREWORKS_ANGLE, MAX_FIREWORKS_ANGLE);
}

inline auto FlyingStarsFx::FlyingStarsImpl::GetFountainBombAngle(const Star& star) const -> float
{
  constexpr float MIN_FOUNTAIN_ANGLE = m_pi + 0.1F;
  constexpr float MAX_MIN_FOUNTAIN_ANGLE = m_pi + m_third_pi;
  constexpr float MAX_FOUNTAIN_ANGLE = m_two_pi - 0.1F;

  const float xFactor = star.pos.x / m_xMax;
  const float minAngle =
      stdnew::lerp(MIN_FOUNTAIN_ANGLE, MAX_MIN_FOUNTAIN_ANGLE - 0.1F, 1.0F - xFactor);
  const float maxAngle = stdnew::lerp(MAX_MIN_FOUNTAIN_ANGLE + 0.1F, MAX_FOUNTAIN_ANGLE, xFactor);

  return m_goomRand.GetRandInRange(minAngle, maxAngle);
}

inline auto FlyingStarsFx::FlyingStarsImpl::GetRainBombAngle(const Star& star) const -> float
{
  constexpr float MIN_RAIN_ANGLE = 0.1F;
  constexpr float MAX_MIN_RAIN_ANGLE = m_third_pi;
  constexpr float MAX_RAIN_ANGLE = m_pi - 0.1F;

  const float xFactor = star.pos.x / m_xMax;
  const float minAngle = stdnew::lerp(MIN_RAIN_ANGLE, MAX_MIN_RAIN_ANGLE - 0.1F, 1.0F - xFactor);
  const float maxAngle = stdnew::lerp(MAX_MIN_RAIN_ANGLE + 0.1F, MAX_RAIN_ANGLE, xFactor);

  return m_goomRand.GetRandInRange(minAngle, maxAngle);
}

#if __cplusplus <= 201402L
} // namespace VISUAL_FX
} // namespace GOOM
#else
} // namespace GOOM::VISUAL_FX
#endif
