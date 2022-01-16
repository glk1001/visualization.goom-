#include "goom_dots_fx.h"

#include "color/colormaps.h"
#include "color/colorutils.h"
#include "color/random_colormaps.h"
#include "color/random_colormaps_manager.h"
#include "draw/goom_draw.h"
#include "fx_helpers.h"
//#undef NO_LOGGING
#include "goom/logging.h"
#include "goom/spimpl.h"
#include "goom_graphic.h"
#include "goom_plugin_info.h"
#include "utils/goom_rand_base.h"
#include "utils/graphics/image_bitmaps.h"
#include "utils/graphics/small_image_bitmaps.h"
#include "v2d.h"

#include <cmath>
#include <cstdint>
#include <memory>

namespace GOOM::VISUAL_FX
{

using COLOR::ColorMapGroup;
using COLOR::GetBrighterColor;
using COLOR::GetIncreasedChroma;
using COLOR::RandomColorMaps;
using COLOR::RandomColorMapsManager;
using DRAW::IGoomDraw;
using UTILS::IGoomRand;
using UTILS::ImageBitmap;
using UTILS::SmallImageBitmaps;
using UTILS::Weights;

class GoomDotsFx::GoomDotsFxImpl
{
public:
  GoomDotsFxImpl(const FxHelpers& fxHelpers, const SmallImageBitmaps& smallBitmaps) noexcept;

  void Start();

  void SetWeightedColorMaps(uint32_t dotNum, std::shared_ptr<RandomColorMaps> weightedMaps);

  void ApplySingle();
  void ApplyMultiple();

private:
  IGoomDraw& m_draw;
  const PluginInfo& m_goomInfo;
  const IGoomRand& m_goomRand;
  const SmallImageBitmaps& m_smallBitmaps;
  const V2dInt m_screenMidPoint;
  const uint32_t m_pointWidth;
  const uint32_t m_pointHeight;

  const float m_pointWidthDiv2;
  const float m_pointHeightDiv2;
  const float m_pointWidthDiv3;
  const float m_pointHeightDiv3;

  SmallImageBitmaps::ImageNames m_currentBitmapName{};
  static constexpr uint32_t MAX_FLOWERS_IN_ROW = 100;
  uint32_t m_numFlowersInRow = 0;
  const Weights<SmallImageBitmaps::ImageNames> m_flowerDotTypes;
  [[nodiscard]] auto GetImageBitmap(size_t size) const -> const ImageBitmap&;
  void SetFlowerBitmap();
  void SetNonFlowerBitmap();
  void SetNextCurrentBitmapName();
  [[nodiscard]] auto ChangeDotColorsEvent() const -> bool;

  static constexpr size_t MIN_DOT_SIZE = 5;
  static constexpr size_t MAX_DOT_SIZE = 17;
  static_assert(MAX_DOT_SIZE <= SmallImageBitmaps::MAX_IMAGE_SIZE, "Max dot size mismatch.");

  std::array<std::shared_ptr<RandomColorMaps>, NUM_DOT_TYPES> m_colorMaps{};
  std::array<RandomColorMapsManager, NUM_DOT_TYPES> m_colorMapsManagers{};
  std::array<uint32_t, NUM_DOT_TYPES> m_colorMapIds{};
  std::array<bool, NUM_DOT_TYPES> m_usePrimaryColors{};
  Pixel m_middleColor{};
  bool m_useSingleBufferOnly = true;
  bool m_thereIsOneBuffer = true;
  bool m_useIncreasedChroma = true;
  bool m_useMiddleColor = true;
  [[nodiscard]] auto GetDotColor(size_t dotNum, float t) const -> Pixel;
  [[nodiscard]] static auto GetDotPrimaryColor(size_t dotNum) -> Pixel;
  [[nodiscard]] static auto IsImagePointCloseToMiddle(size_t x, size_t y, uint32_t radius) -> bool;
  [[nodiscard]] static auto GetMargin(uint32_t radius) -> size_t;
  [[nodiscard]] auto GetMiddleColor() const -> Pixel;

  uint32_t m_loopVar = 0; // mouvement des points

  void Update();

  void ChangeColors();
  [[nodiscard]] static auto GetLargeSoundFactor(const SoundInfo& soundInfo) -> float;

  void DotFilter(const Pixel& color, const V2dInt& dotPosition, uint32_t radius);
  [[nodiscard]] auto GetDotPosition(float xOffsetAmp,
                                    float yOffsetAmp,
                                    float xOffsetFreqDenom,
                                    float yOffsetFreqDenom,
                                    uint32_t offsetCycle) const -> V2dInt;

  static constexpr float GAMMA = 2.0F;
  static constexpr float GAMMA_BRIGHTNESS_THRESHOLD = 0.01F;
  const COLOR::GammaCorrection m_gammaCorrect{GAMMA, GAMMA_BRIGHTNESS_THRESHOLD};
  auto GetGammaCorrection(float brightness, const Pixel& color) const -> Pixel;
};

GoomDotsFx::GoomDotsFx(const FxHelpers& fxHelpers, const SmallImageBitmaps& smallBitmaps) noexcept
  : m_fxImpl{spimpl::make_unique_impl<GoomDotsFxImpl>(fxHelpers, smallBitmaps)}
{
}

void GoomDotsFx::SetWeightedColorMaps(const uint32_t dotNum,
                                      const std::shared_ptr<RandomColorMaps> weightedMaps)
{
  m_fxImpl->SetWeightedColorMaps(dotNum, weightedMaps);
}

void GoomDotsFx::Start()
{
  m_fxImpl->Start();
}

void GoomDotsFx::Resume()
{
  // nothing to do
}

void GoomDotsFx::Suspend()
{
  // nothing to do
}

void GoomDotsFx::Finish()
{
  // nothing to do
}

auto GoomDotsFx::GetFxName() const -> std::string
{
  return "goom dots";
}

void GoomDotsFx::ApplySingle()
{
  m_fxImpl->ApplySingle();
}

void GoomDotsFx::ApplyMultiple()
{
  m_fxImpl->ApplyMultiple();
}

constexpr float HALF = 1.0F / 2.0F;
constexpr float THIRD = 1.0F / 3.0F;

// clang-format off
constexpr float IMAGE_NAMES_ORANGE_FLOWER_WEIGHT = 10.0F;
constexpr float IMAGE_NAMES_PINK_FLOWER_WEIGHT   =  5.0F;
constexpr float IMAGE_NAMES_RED_FLOWER_WEIGHT    = 10.0F;
constexpr float IMAGE_NAMES_WHITE_FLOWER_WEIGHT  =  5.0F;
// clang-format on

GoomDotsFx::GoomDotsFxImpl::GoomDotsFxImpl(const FxHelpers& fxHelpers,
                                           const SmallImageBitmaps& smallBitmaps) noexcept
  : m_draw{fxHelpers.GetDraw()},
    m_goomInfo{fxHelpers.GetGoomInfo()},
    m_goomRand{fxHelpers.GetGoomRand()},
    m_smallBitmaps{smallBitmaps},
    m_screenMidPoint{m_goomInfo.GetScreenInfo().width / 2, m_goomInfo.GetScreenInfo().height / 2},
    m_pointWidth{(m_goomInfo.GetScreenInfo().width * 2) / 5},
    m_pointHeight{(m_goomInfo.GetScreenInfo().height * 2) / 5},
    m_pointWidthDiv2{HALF * static_cast<float>(m_pointWidth)},
    m_pointHeightDiv2{HALF * static_cast<float>(m_pointHeight)},
    m_pointWidthDiv3{THIRD * static_cast<float>(m_pointWidth)},
    m_pointHeightDiv3{THIRD * static_cast<float>(m_pointHeight)},
    // clang-format off
    m_flowerDotTypes{
        m_goomRand,
        {
            {SmallImageBitmaps::ImageNames::ORANGE_FLOWER, IMAGE_NAMES_ORANGE_FLOWER_WEIGHT},
            {SmallImageBitmaps::ImageNames::PINK_FLOWER,   IMAGE_NAMES_PINK_FLOWER_WEIGHT},
            {SmallImageBitmaps::ImageNames::RED_FLOWER,    IMAGE_NAMES_RED_FLOWER_WEIGHT},
            {SmallImageBitmaps::ImageNames::WHITE_FLOWER,  IMAGE_NAMES_WHITE_FLOWER_WEIGHT},
        }
    }
// clang-format on
{
}

inline auto GoomDotsFx::GoomDotsFxImpl::ChangeDotColorsEvent() const -> bool
{
  constexpr float PROB_CHANGE_DOT_COLORS = 0.33F;
  return m_goomRand.ProbabilityOf(PROB_CHANGE_DOT_COLORS);
}

inline void GoomDotsFx::GoomDotsFxImpl::Start()
{
  ChangeColors();
}

inline auto GoomDotsFx::GoomDotsFxImpl::GetImageBitmap(const size_t size) const
    -> const ImageBitmap&
{
  return m_smallBitmaps.GetImageBitmap(m_currentBitmapName,
                                       std::clamp(size, MIN_DOT_SIZE, MAX_DOT_SIZE));
}

inline void GoomDotsFx::GoomDotsFxImpl::ChangeColors()
{
  for (auto& colorMapsManager : m_colorMapsManagers)
  {
    colorMapsManager.ChangeAllColorMapsNow();
  }

  for (auto& usePrimaryColor : m_usePrimaryColors)
  {
    constexpr float PROB_USE_PRIMARY_COLOR = 0.5F;
    usePrimaryColor = m_goomRand.ProbabilityOf(PROB_USE_PRIMARY_COLOR);
  }

  constexpr float PROB_USE_SINGLE_BUFFER_ONLY = 0.0F / 2.0F;
  m_useSingleBufferOnly = m_goomRand.ProbabilityOf(PROB_USE_SINGLE_BUFFER_ONLY);

  constexpr float PROB_INCREASED_CHROMA = 0.8F;
  m_useIncreasedChroma = m_goomRand.ProbabilityOf(PROB_INCREASED_CHROMA);

  constexpr float PROB_USE_MIDDLE_COLOR = 0.05F;
  m_useMiddleColor = m_goomRand.ProbabilityOf(PROB_USE_MIDDLE_COLOR);
  if (m_useMiddleColor)
  {
    m_middleColor = GetMiddleColor();
  }
}

auto GoomDotsFx::GoomDotsFxImpl::GetMiddleColor() const -> Pixel
{
  constexpr float PROB_PRIMARY_COLOR = 0.5F;
  if (m_goomRand.ProbabilityOf(PROB_PRIMARY_COLOR))
  {
    return GetDotPrimaryColor(m_goomRand.GetRandInRange(0U, NUM_DOT_TYPES));
  }

  constexpr float MIN_MIX_T = 0.1F;
  constexpr float MAX_MIX_T = 1.0F;
  return RandomColorMaps{m_goomRand}.GetRandomColor(
      *m_colorMaps[0]->GetRandomColorMapPtr(ColorMapGroup::MISC,
                                            RandomColorMaps::ALL_COLOR_MAP_TYPES),
      MIN_MIX_T, MAX_MIX_T);
}

inline void GoomDotsFx::GoomDotsFxImpl::SetWeightedColorMaps(
    const uint32_t dotNum, const std::shared_ptr<RandomColorMaps> weightedMaps)
{
  m_colorMaps.at(dotNum) = weightedMaps;
  m_colorMapIds.at(dotNum) = m_colorMapsManagers.at(dotNum).AddColorMapInfo(
      {m_colorMaps.at(dotNum),
       m_colorMaps.at(dotNum)->GetRandomColorMapName(m_colorMaps.at(dotNum)->GetRandomGroup()),
       RandomColorMaps::ALL_COLOR_MAP_TYPES});
}

inline void GoomDotsFx::GoomDotsFxImpl::ApplySingle()
{
  m_thereIsOneBuffer = true;
  Update();
}

inline void GoomDotsFx::GoomDotsFxImpl::ApplyMultiple()
{
  m_thereIsOneBuffer = false;
  Update();
}

void GoomDotsFx::GoomDotsFxImpl::Update()
{
  uint32_t radius = MIN_DOT_SIZE / 2;
  if ((0 == m_goomInfo.GetSoundInfo().GetTimeSinceLastGoom()) || ChangeDotColorsEvent())
  {
    ChangeColors();
    radius = m_goomRand.GetRandInRange(radius, (MAX_DOT_SIZE / 2) + 1);
    SetNextCurrentBitmapName();
  }

  const float speedFactor = 0.35F * m_goomInfo.GetSoundInfo().GetSpeed();
  const float largeFactor = GetLargeSoundFactor(m_goomInfo.GetSoundInfo());
  const auto speedVarMult80Plus15 = static_cast<uint32_t>((speedFactor * 80.0F) + 15.0F);
  const auto speedVarMult50Plus1 = static_cast<uint32_t>((speedFactor * 50.0F) + 1.0F);

  const float pointWidthDiv2MultLarge = m_pointWidthDiv2 * largeFactor;
  const float pointHeightDiv2MultLarge = m_pointHeightDiv2 * largeFactor;
  const float pointWidthDiv3MultLarge = (m_pointWidthDiv3 + 5.0F) * largeFactor;
  const float pointHeightDiv3MultLarge = (m_pointHeightDiv3 + 5.0F) * largeFactor;
  const float pointWidthMultLarge = static_cast<float>(m_pointWidth) * largeFactor;
  const float pointHeightMultLarge = static_cast<float>(m_pointHeight) * largeFactor;

  const float dot0XOffsetAmp = ((static_cast<float>(m_pointWidth) - 6.0F) * largeFactor) + 5.0F;
  const float dot0YOffsetAmp = ((static_cast<float>(m_pointHeight) - 6.0F) * largeFactor) + 5.0F;
  const float dot3XOffsetAmp = (m_pointHeightDiv3 * largeFactor) + 20.0F;
  const float dot3YOffsetAmp = dot3XOffsetAmp;

  const size_t speedVarMult80Plus15Div15 = speedVarMult80Plus15 / 15;
  constexpr float T_MIN = 0.1F;
  constexpr float T_MAX = 1.0F;
  const float tStep = (T_MAX - T_MIN) / static_cast<float>(speedVarMult80Plus15Div15);

  float t = T_MIN;
  for (uint32_t i = 1; i <= speedVarMult80Plus15Div15; ++i)
  {
    m_loopVar += speedVarMult50Plus1;

    const uint32_t loopVarDivI = m_loopVar / i;
    const float iMult10 = 10.0F * static_cast<float>(i);

    const Pixel dot0Color = GetDotColor(0, t);
    const float dot0XFreqDenom = static_cast<float>(i) * 152.0F;
    const float dot0YFreqDenom = 128.0F;
    const uint32_t dot0Cycle = m_loopVar + (i * 2032);
    const V2dInt dot0Position =
        GetDotPosition(dot0XOffsetAmp, dot0YOffsetAmp, dot0XFreqDenom, dot0YFreqDenom, dot0Cycle);

    const Pixel dot1Color = GetDotColor(1, t);
    const float dot1XOffsetAmp = (pointWidthDiv2MultLarge / static_cast<float>(i)) + iMult10;
    const float dot1YOffsetAmp = (pointHeightDiv2MultLarge / static_cast<float>(i)) + iMult10;
    const float dot1XFreqDenom = 96.0F;
    const float dot1YFreqDenom = static_cast<float>(i) * 80.0F;
    const uint32_t dot1Cycle = loopVarDivI;
    const V2dInt dot1Position =
        GetDotPosition(dot1XOffsetAmp, dot1YOffsetAmp, dot1XFreqDenom, dot1YFreqDenom, dot1Cycle);

    const Pixel dot2Color = GetDotColor(2, t);
    const float dot2XOffsetAmp = (pointWidthDiv3MultLarge / static_cast<float>(i)) + iMult10;
    const float dot2YOffsetAmp = (pointHeightDiv3MultLarge / static_cast<float>(i)) + iMult10;
    const float dot2XFreqDenom = static_cast<float>(i) + 122.0F;
    const float dot2YFreqDenom = 134.0F;
    const uint32_t dot2Cycle = loopVarDivI;
    const V2dInt dot2Position =
        GetDotPosition(dot2XOffsetAmp, dot2YOffsetAmp, dot2XFreqDenom, dot2YFreqDenom, dot2Cycle);

    const Pixel dot3Color = GetDotColor(3, t);
    const float dot3XFreqDenom = 58.0F;
    const float dot3YFreqDenom = static_cast<float>(i) * 66.0F;
    const uint32_t dot3Cycle = loopVarDivI;
    const V2dInt dot3Position =
        GetDotPosition(dot3XOffsetAmp, dot3YOffsetAmp, dot3XFreqDenom, dot3YFreqDenom, dot3Cycle);

    const Pixel dot4Color = GetDotColor(4, t);
    const float dot4XOffsetAmp = (pointWidthMultLarge + iMult10) / static_cast<float>(i);
    const float dot4YOffsetAmp = (pointHeightMultLarge + iMult10) / static_cast<float>(i);
    const float dot4XFreqDenom = 66.0F;
    const float dot4YFreqDenom = 74.0F;
    const uint32_t dot4Cycle = m_loopVar + (i * 500);
    const V2dInt dot4Position =
        GetDotPosition(dot4XOffsetAmp, dot4YOffsetAmp, dot4XFreqDenom, dot4YFreqDenom, dot4Cycle);

    DotFilter(dot0Color, dot0Position, radius);
    DotFilter(dot1Color, dot1Position, radius);
    DotFilter(dot2Color, dot2Position, radius);
    DotFilter(dot3Color, dot3Position, radius);
    DotFilter(dot4Color, dot4Position, radius);

    t += tStep;
  }
}

inline auto GoomDotsFx::GoomDotsFxImpl::GetDotColor(const size_t dotNum, const float t) const
    -> Pixel
{
  if (m_usePrimaryColors.at(dotNum))
  {
    return GetDotPrimaryColor(dotNum);
  }

  return m_colorMapsManagers.at(dotNum).GetColorMap(m_colorMapIds.at(dotNum)).GetColor(t);
}

inline auto GoomDotsFx::GoomDotsFxImpl::GetDotPrimaryColor(const size_t dotNum) -> Pixel
{
  static const std::array s_PRIMARY_COLORS{
      Pixel{255,   0,   0, 0},
      Pixel{  0, 255,   0, 0},
      Pixel{  0,   0, 255, 0},
      Pixel{255, 255,   0, 0},
      Pixel{  0, 255, 255, 0},
  };
  return s_PRIMARY_COLORS.at(dotNum);
}

inline void GoomDotsFx::GoomDotsFxImpl::SetNextCurrentBitmapName()
{
  constexpr float PROB_MORE_FLOWERS = 1.0F / 50.0F;

  if (m_numFlowersInRow > 0)
  {
    SetFlowerBitmap();
  }
  else if (m_goomRand.ProbabilityOf(PROB_MORE_FLOWERS))
  {
    m_numFlowersInRow = 1;
    SetNextCurrentBitmapName();
  }
  else
  {
    SetNonFlowerBitmap();
  }
}

inline void GoomDotsFx::GoomDotsFxImpl::SetFlowerBitmap()
{
  ++m_numFlowersInRow;
  if (m_numFlowersInRow > MAX_FLOWERS_IN_ROW)
  {
    m_numFlowersInRow = 0;
  }

  m_currentBitmapName = m_flowerDotTypes.GetRandomWeighted();
}

inline void GoomDotsFx::GoomDotsFxImpl::SetNonFlowerBitmap()
{
  constexpr float PROB_SPHERE = 0.7F;

  if (m_goomRand.ProbabilityOf(PROB_SPHERE))
  {
    m_currentBitmapName = SmallImageBitmaps::ImageNames::SPHERE;
  }
  else
  {
    m_currentBitmapName = SmallImageBitmaps::ImageNames::CIRCLE;
  }
}

inline auto GoomDotsFx::GoomDotsFxImpl::GetLargeSoundFactor(const SoundInfo& soundInfo) -> float
{
  constexpr float SOUND_SPEED_REDUCER = 1.0F / 50.0F;
  constexpr float VOLUME_REDUCER = 1.0F / 1.5F;
  return (SOUND_SPEED_REDUCER * soundInfo.GetSpeed()) + (VOLUME_REDUCER * soundInfo.GetVolume());
}

inline auto GoomDotsFx::GoomDotsFxImpl::GetDotPosition(const float xOffsetAmp,
                                                       const float yOffsetAmp,
                                                       const float xOffsetFreqDenom,
                                                       const float yOffsetFreqDenom,
                                                       const uint32_t offsetCycle) const -> V2dInt
{
  const auto xOffset = static_cast<int32_t>(
      xOffsetAmp * std::cos(static_cast<float>(offsetCycle) / xOffsetFreqDenom));
  const auto yOffset = static_cast<int32_t>(
      yOffsetAmp * std::sin(static_cast<float>(offsetCycle) / yOffsetFreqDenom));

  return {m_screenMidPoint.x + xOffset, m_screenMidPoint.y + yOffset};
}

void GoomDotsFx::GoomDotsFxImpl::DotFilter(const Pixel& color,
                                           const V2dInt& dotPosition,
                                           const uint32_t radius)
{
  const uint32_t diameter = (2 * radius) + 1; // must be odd
  const auto screenWidthLessDiameter =
      static_cast<int32_t>(m_goomInfo.GetScreenInfo().width - diameter);
  const auto screenHeightLessDiameter =
      static_cast<int32_t>(m_goomInfo.GetScreenInfo().height - diameter);

  if ((dotPosition.x < static_cast<int32_t>(diameter)) ||
      (dotPosition.y < static_cast<int32_t>(diameter)) ||
      (dotPosition.x >= screenWidthLessDiameter) || (dotPosition.y >= screenHeightLessDiameter))
  {
    return;
  }

  constexpr float BRIGHTNESS = 3.5F;
  const auto getColor1 = [&](const size_t x, const size_t y, const Pixel& bgnd)
  {
    if (0 == bgnd.A())
    {
      return Pixel::BLACK;
    }
    const Pixel newColor =
        m_useMiddleColor && IsImagePointCloseToMiddle(x, y, radius) ? m_middleColor : color;
    constexpr float COLOR_MIX_T = 0.6F;
    const Pixel mixedColor = COLOR::IColorMap::GetColorMix(bgnd, newColor, COLOR_MIX_T);
    const Pixel finalColor = (!m_useIncreasedChroma) ? mixedColor : GetIncreasedChroma(mixedColor);
    return GetGammaCorrection(BRIGHTNESS, finalColor);
  };
  const auto getColor2 = [&]([[maybe_unused]] const size_t x, [[maybe_unused]] const size_t y,
                             [[maybe_unused]] const Pixel& bgnd) { return getColor1(x, y, bgnd); };

  const auto xMid = dotPosition.x + static_cast<int32_t>(radius);
  const auto yMid = dotPosition.y + static_cast<int32_t>(radius);
  if (m_thereIsOneBuffer || m_useSingleBufferOnly)
  {
    m_draw.Bitmap(xMid, yMid, GetImageBitmap(diameter), getColor1);
  }
  else
  {
    m_draw.Bitmap(xMid, yMid, GetImageBitmap(diameter), {getColor1, getColor2});
  }
}

inline auto GoomDotsFx::GoomDotsFxImpl::IsImagePointCloseToMiddle(const size_t x,
                                                                  const size_t y,
                                                                  const uint32_t radius) -> bool
{
  const size_t margin = GetMargin(radius);
  const size_t minVal = radius - margin;
  const size_t maxVal = radius + margin;
  return (minVal <= x) && (x <= maxVal) && (minVal <= y) && (y <= maxVal);
}

inline auto GoomDotsFx::GoomDotsFxImpl::GetMargin(const uint32_t radius) -> size_t
{
  constexpr size_t SMALLEST_MARGIN = 2;
  constexpr uint32_t SMALL_RADIUS = 7;

  if (radius < SMALL_RADIUS)
  {
    return SMALLEST_MARGIN;
  }
  return SMALLEST_MARGIN + 1;
}

inline auto GoomDotsFx::GoomDotsFxImpl::GetGammaCorrection(const float brightness,
                                                           const Pixel& color) const -> Pixel
{
  if constexpr (1.0F == GAMMA)
  {
    return GetBrighterColor(brightness, color);
  }
  return m_gammaCorrect.GetCorrection(brightness, color);
}

} // namespace GOOM::VISUAL_FX
