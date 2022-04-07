#include "goom_dots_fx.h"

//#undef NO_LOGGING

#include "color/colormaps.h"
#include "color/colorutils.h"
#include "color/random_colormaps.h"
#include "color/random_colormaps_manager.h"
#include "draw/goom_draw.h"
#include "fx_helper.h"
#include "goom/logging.h"
#include "goom/spimpl.h"
#include "goom_graphic.h"
#include "goom_plugin_info.h"
#include "point2d.h"
#include "utils/graphics/image_bitmaps.h"
#include "utils/graphics/small_image_bitmaps.h"
#include "utils/math/goom_rand_base.h"
#include "utils/math/misc.h"
#include "utils/math/paths.h"
#include "utils/t_values.h"

#include <array>
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
using UTILS::Logging;
using UTILS::TValue;
using UTILS::GRAPHICS::ImageBitmap;
using UTILS::GRAPHICS::SmallImageBitmaps;
using UTILS::MATH::Epicycloid;
using UTILS::MATH::Fraction;
using UTILS::MATH::HALF;
using UTILS::MATH::Hypotrochoid;
using UTILS::MATH::IGoomRand;
using UTILS::MATH::IPath;
using UTILS::MATH::LissajousPath;
using UTILS::MATH::S_HALF;
using UTILS::MATH::THIRD;
using UTILS::MATH::U_HALF;
using UTILS::MATH::Weights;

class GoomDotsFx::GoomDotsFxImpl
{
public:
  GoomDotsFxImpl(const FxHelper& fxHelper, const SmallImageBitmaps& smallBitmaps);

  void Start();

  void SetWeightedColorMaps(uint32_t dotNum, std::shared_ptr<RandomColorMaps> weightedMaps);

  void ApplySingle();
  void ApplyMultiple();

private:
  IGoomDraw& m_draw;
  const PluginInfo& m_goomInfo;
  const IGoomRand& m_goomRand;
  const SmallImageBitmaps& m_smallBitmaps;
  const Point2dInt m_screenMidpoint;

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

  std::array<TValue, NUM_DOT_TYPES> m_dotPositionTs;
  [[nodiscard]] static auto GetDotPositionTs() -> std::array<TValue, NUM_DOT_TYPES>;
  const std::array<std::unique_ptr<IPath>, NUM_DOT_TYPES> m_dotPaths;
  [[nodiscard]] static auto GetDotPaths(const Point2dInt& centre,
                                        std::array<TValue, NUM_DOT_TYPES>& dotPositionTs)
      -> std::array<std::unique_ptr<IPath>, NUM_DOT_TYPES>;

  void Update();

  void ChangeColors();

  void DotFilter(const Pixel& color, const Point2dInt& dotPosition, uint32_t radius);

  static constexpr float GAMMA = 2.0F;
  static constexpr float GAMMA_BRIGHTNESS_THRESHOLD = 0.01F;
  const COLOR::GammaCorrection m_gammaCorrect{GAMMA, GAMMA_BRIGHTNESS_THRESHOLD};
};

GoomDotsFx::GoomDotsFx(const FxHelper& fxHelper, const SmallImageBitmaps& smallBitmaps) noexcept
  : m_fxImpl{spimpl::make_unique_impl<GoomDotsFxImpl>(fxHelper, smallBitmaps)}
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

// clang-format off
static constexpr float IMAGE_NAMES_ORANGE_FLOWER_WEIGHT = 10.0F;
static constexpr float IMAGE_NAMES_PINK_FLOWER_WEIGHT   =  5.0F;
static constexpr float IMAGE_NAMES_RED_FLOWER_WEIGHT    = 10.0F;
static constexpr float IMAGE_NAMES_WHITE_FLOWER_WEIGHT  =  5.0F;
// clang-format on

GoomDotsFx::GoomDotsFxImpl::GoomDotsFxImpl(const FxHelper& fxHelper,
                                           const SmallImageBitmaps& smallBitmaps)
  : m_draw{fxHelper.GetDraw()},
    m_goomInfo{fxHelper.GetGoomInfo()},
    m_goomRand{fxHelper.GetGoomRand()},
    m_smallBitmaps{smallBitmaps},
    m_screenMidpoint{U_HALF * m_goomInfo.GetScreenInfo().width,
                     U_HALF * m_goomInfo.GetScreenInfo().height},
    // clang-format off
    m_flowerDotTypes{
        m_goomRand,
        {
            {SmallImageBitmaps::ImageNames::ORANGE_FLOWER, IMAGE_NAMES_ORANGE_FLOWER_WEIGHT},
            {SmallImageBitmaps::ImageNames::PINK_FLOWER,   IMAGE_NAMES_PINK_FLOWER_WEIGHT},
            {SmallImageBitmaps::ImageNames::RED_FLOWER,    IMAGE_NAMES_RED_FLOWER_WEIGHT},
            {SmallImageBitmaps::ImageNames::WHITE_FLOWER,  IMAGE_NAMES_WHITE_FLOWER_WEIGHT},
        }
    },
    // clang-format on
    m_dotPositionTs{GetDotPositionTs()},
    m_dotPaths{GetDotPaths(m_screenMidpoint, m_dotPositionTs)}
{
}

auto GoomDotsFx::GoomDotsFxImpl::GetDotPositionTs() -> std::array<TValue, NUM_DOT_TYPES>
{
  static constexpr float HYPOTROCHOID_STEP_SIZE = 0.01F;
  static constexpr float LISSAJOUS_STEP_SIZE = 0.01F;
  static constexpr float EPICYCLOID_STEP_SIZE = 0.001F;

  return {
      {
       {TValue::StepType::CONTINUOUS_REVERSIBLE, HYPOTROCHOID_STEP_SIZE},
       {TValue::StepType::CONTINUOUS_REVERSIBLE, HYPOTROCHOID_STEP_SIZE},
       {TValue::StepType::CONTINUOUS_REVERSIBLE, HYPOTROCHOID_STEP_SIZE},
       {TValue::StepType::CONTINUOUS_REVERSIBLE, LISSAJOUS_STEP_SIZE},
       {TValue::StepType::CONTINUOUS_REVERSIBLE, EPICYCLOID_STEP_SIZE},
       }
  };
}

auto GoomDotsFx::GoomDotsFxImpl::GetDotPaths(const Point2dInt& centre,
                                             std::array<TValue, NUM_DOT_TYPES>& dotPositionTs)
    -> std::array<std::unique_ptr<IPath>, NUM_DOT_TYPES>
{
  static constexpr Hypotrochoid::Params HYPOTROCHOID_PARAMS1{7.0F, 3.0F, 5.0F, 30.0F};
  static constexpr Hypotrochoid::Params HYPOTROCHOID_PARAMS2{8.0F, 3.0F, 5.0F, 30.0F};
  static constexpr Hypotrochoid::Params HYPOTROCHOID_PARAMS3{9.0F, 3.0F, 5.0F, 30.0F};
  static constexpr LissajousPath::Params LISSAJOUS_PATH_PARAMS{50.0F, 50.F, 3.0F, 2.0F};
  static constexpr Epicycloid::Params EPICYCLOID_PARAMS{5.1F, 1.0F, 30.0F};

  return {
      {
       std::make_unique<Hypotrochoid>(centre, dotPositionTs.at(0), HYPOTROCHOID_PARAMS1),
       std::make_unique<Hypotrochoid>(centre, dotPositionTs.at(1), HYPOTROCHOID_PARAMS2),
       std::make_unique<Hypotrochoid>(centre, dotPositionTs.at(2), HYPOTROCHOID_PARAMS3),
       std::make_unique<LissajousPath>(centre, dotPositionTs.at(3), LISSAJOUS_PATH_PARAMS),
       std::make_unique<Epicycloid>(centre, dotPositionTs.at(4), EPICYCLOID_PARAMS),
       }
  };
}

inline auto GoomDotsFx::GoomDotsFxImpl::ChangeDotColorsEvent() const -> bool
{
  static constexpr float PROB_CHANGE_DOT_COLORS = 0.33F;
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
    static constexpr float PROB_USE_PRIMARY_COLOR = 0.5F;
    usePrimaryColor = m_goomRand.ProbabilityOf(PROB_USE_PRIMARY_COLOR);
  }

  static constexpr float PROB_USE_SINGLE_BUFFER_ONLY = 0.0F / 2.0F;
  m_useSingleBufferOnly = m_goomRand.ProbabilityOf(PROB_USE_SINGLE_BUFFER_ONLY);

  static constexpr float PROB_INCREASED_CHROMA = 0.8F;
  m_useIncreasedChroma = m_goomRand.ProbabilityOf(PROB_INCREASED_CHROMA);

  static constexpr float PROB_USE_MIDDLE_COLOR = 0.05F;
  m_useMiddleColor = m_goomRand.ProbabilityOf(PROB_USE_MIDDLE_COLOR);
  if (m_useMiddleColor)
  {
    m_middleColor = GetMiddleColor();
  }
}

auto GoomDotsFx::GoomDotsFxImpl::GetMiddleColor() const -> Pixel
{
  if (constexpr float PROB_PRIMARY_COLOR = 0.1F; m_goomRand.ProbabilityOf(PROB_PRIMARY_COLOR))
  {
    return GetDotPrimaryColor(m_goomRand.GetRandInRange(0U, NUM_DOT_TYPES));
  }

  static constexpr float MIN_MIX_T = 0.1F;
  static constexpr float MAX_MIX_T = 1.0F;
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
    radius = m_goomRand.GetRandInRange(radius, static_cast<uint32_t>(S_HALF * MAX_DOT_SIZE) + 1);
    SetNextCurrentBitmapName();
  }

  const float speedFactor = 0.35F * m_goomInfo.GetSoundInfo().GetSpeed();
  //  const float largeFactor = GetLargeSoundFactor(m_goomInfo.GetSoundInfo());
  const auto speedVarMult80Plus15 = static_cast<uint32_t>((speedFactor * 80.0F) + 15.0F);

  const size_t speedVarMult80Plus15Div15 = speedVarMult80Plus15 / 15;
  static constexpr float T_MIN = 0.1F;
  static constexpr float T_MAX = 1.0F;
  const float tStep = (T_MAX - T_MIN) / static_cast<float>(speedVarMult80Plus15Div15);

  float t = T_MIN;
  for (uint32_t i = 1; i <= speedVarMult80Plus15Div15; ++i)
  {
    for (size_t dotNum = 0; dotNum < NUM_DOT_TYPES; ++dotNum)
    {
      const Pixel dotColor = GetDotColor(dotNum, t);
      const Point2dInt dotPosition = m_dotPaths.at(dotNum)->GetNextPoint();
      m_dotPaths.at(dotNum)->IncrementT();
      DotFilter(dotColor, dotPosition, radius);
    }

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
  static constexpr float PROB_MORE_FLOWERS = 1.0F / 50.0F;

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
  static constexpr float PROB_SPHERE = 0.7F;

  if (m_goomRand.ProbabilityOf(PROB_SPHERE))
  {
    m_currentBitmapName = SmallImageBitmaps::ImageNames::SPHERE;
  }
  else
  {
    m_currentBitmapName = SmallImageBitmaps::ImageNames::CIRCLE;
  }
}

void GoomDotsFx::GoomDotsFxImpl::DotFilter(const Pixel& color,
                                           const Point2dInt& dotPosition,
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

  static constexpr float BRIGHTNESS = 3.5F;
  const auto getColor1 = [this, &radius, &color](const size_t x, const size_t y, const Pixel& bgnd)
  {
    if (0 == bgnd.A())
    {
      return Pixel::BLACK;
    }
    const Pixel newColor =
        m_useMiddleColor && IsImagePointCloseToMiddle(x, y, radius) ? m_middleColor : color;
    static constexpr float COLOR_MIX_T = 0.6F;
    const Pixel mixedColor = COLOR::IColorMap::GetColorMix(bgnd, newColor, COLOR_MIX_T);
    const Pixel finalColor = (!m_useIncreasedChroma) ? mixedColor : GetIncreasedChroma(mixedColor);
    return m_gammaCorrect.GetCorrection(BRIGHTNESS, finalColor);
  };
  const auto getColor2 =
      [&getColor1]([[maybe_unused]] const size_t x, [[maybe_unused]] const size_t y,
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
  static constexpr size_t SMALLEST_MARGIN = 2;
  if (constexpr uint32_t SMALL_RADIUS = 7; radius < SMALL_RADIUS)
  {
    return SMALLEST_MARGIN;
  }
  return SMALLEST_MARGIN + 1;
}

} // namespace GOOM::VISUAL_FX
