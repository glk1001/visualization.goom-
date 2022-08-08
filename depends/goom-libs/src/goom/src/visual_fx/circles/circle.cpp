#include "circle.h"

//#undef NO_LOGGING

#include "color/color_maps.h"
#include "color/color_utils.h"
#include "color/random_color_maps.h"
#include "color/random_color_maps_groups.h"
#include "draw/goom_draw.h"
#include "goom_config.h"
#include "goom_graphic.h"
#include "logging.h"
#include "point2d.h"
#include "utils/enum_utils.h"
#include "utils/math/goom_rand_base.h"
#include "utils/math/misc.h"
#include "utils/math/paths.h"
#include "utils/t_values.h"

#include <cmath>
#include <cstdint>
#include <memory>
#include <vector>

namespace GOOM::VISUAL_FX::CIRCLES
{

using COLOR::GetBrighterColor;
using COLOR::IColorMap;
using COLOR::RandomColorMaps;
using COLOR::RandomColorMapsGroups;
using DRAW::IGoomDraw;
using DRAW::MultiplePixels;
using UTILS::Logging; // NOLINT(misc-unused-using-decls)
using UTILS::NUM;
using UTILS::TValue;
using UTILS::MATH::AngleParams;
using UTILS::MATH::CirclePath;
using UTILS::MATH::IGoomRand;
using UTILS::MATH::IsEven;
using UTILS::MATH::ModIncrement;
using UTILS::MATH::OscillatingFunction;
using UTILS::MATH::U_HALF;

class Circle::DotDrawer
{
public:
  DotDrawer(IGoomDraw& draw, const IGoomRand& goomRand, const Helper& helper) noexcept;

  auto SetWeightedColorMaps(const RandomColorMaps& weightedMaps) noexcept -> void;

  auto DrawDot(const Point2dInt& pos,
               uint32_t diameter,
               const Pixel& mainColor,
               const Pixel& lowColor) noexcept -> void;

private:
  DRAW::IGoomDraw& m_draw;
  const IGoomRand& m_goomRand;
  const Helper& m_helper;

  static constexpr float MIN_BGND_MIX_T = 0.2F;
  static constexpr float MAX_BGND_MIX_T = 0.8F;
  float m_bgndMainColorMixT;
  float m_bgndLowColorMixT;
  enum class DecorationType
  {
    NO_DECORATION,
    BLACK_LINES,
    BRIGHT_LINES,
    DIFFERENT_COLORS,
    _num // unused, and marks the enum end
  };
  DecorationType m_decorationType;
  Pixel m_differentColor = BLACK_PIXEL;

  bool m_doCircleDotShapes = true;
  const IColorMap* m_outerCircleDotColorMap{};
  static constexpr float MIN_OUTER_CIRCLE_DOT_COLOR_MIX_T        = 0.1F;
  static constexpr float MAX_OUTER_CIRCLE_DOT_COLOR_MIX_T        = 0.9F;
  float m_outerCircleDotColorMix                                 = MIN_OUTER_CIRCLE_DOT_COLOR_MIX_T;
  static constexpr float CIRCLE_DOT_MAIN_COLOR_BRIGHTNESS_FACTOR = 1.0F;
  static constexpr float CIRCLE_DOT_LOW_COLOR_BRIGHTNESS_FACTOR  = 0.1F;

  auto DrawBitmapDot(const Point2dInt& position,
                     uint32_t diameter,
                     const Pixel& mainColor,
                     const Pixel& lowColor) noexcept -> void;
  [[nodiscard]] auto GetRandomDifferentColor(const RandomColorMaps& weightedMaps) const noexcept
      -> Pixel;
  [[nodiscard]] auto GetRandomDecorationType() const noexcept -> DecorationType;
  [[nodiscard]] static auto IsSpecialPoint(size_t x, size_t y, uint32_t diameter) noexcept -> bool;
  [[nodiscard]] auto GetDotMixedColor(size_t x,
                                      size_t y,
                                      uint32_t diameter,
                                      const Pixel& bgnd,
                                      const Pixel& color,
                                      float mixT) const noexcept -> Pixel;
  auto DrawCircleDot(const Point2dInt& centre,
                     uint32_t diameter,
                     const Pixel& mainColor,
                     const Pixel& lowColor,
                     const IColorMap& innerColorMap) noexcept -> void;
  [[nodiscard]] static auto GetCircleColors(float brightness,
                                            const Pixel& mainColor,
                                            const Pixel& lowColor) noexcept -> MultiplePixels;
  [[nodiscard]] static auto GetCircleColorsWithInner(float brightness,
                                                     const Pixel& mainColor,
                                                     const Pixel& lowColor,
                                                     const Pixel& innerColor,
                                                     float innerColorMix) noexcept
      -> MultiplePixels;
};

Circle::Circle(const FxHelper& fxHelper,
               const Helper& helper,
               const Params& circleParams,
               const OscillatingFunction::Params& pathParams) noexcept
  : m_draw{fxHelper.GetDraw()},
    m_goomInfo{fxHelper.GetGoomInfo()},
    m_goomRand{fxHelper.GetGoomRand()},
    m_helper{helper},
    m_circleCentreFixedTarget{circleParams.circleCentreTarget},
    m_dotPaths{m_goomRand,
               NUM_DOTS,
               GetDotStartingPositions(circleParams.circleCentreStart, circleParams.circleRadius),
               circleParams.circleCentreTarget,
               pathParams},
    m_dotDrawer{std::make_unique<DotDrawer>(m_draw, m_goomRand, m_helper)},
    m_mainColorMaps{RandomColorMapsGroups::MakeSharedAllMapsUnweighted(m_goomRand)},
    m_lowColorMaps{m_mainColorMaps},
    m_mainColorMapsGrid{NUM_DOTS,
                        m_mainColorMaps->GetRandomColorMap(),
                        m_dotPaths.GetPositionTRef(),
                        GetVerticalMainColorMaps(),
                        [this](const size_t i) { return GetColorMixT(i); }},
    m_lowColorMapsGrid{NUM_DOTS,
                       m_lowColorMaps->GetRandomColorMap(),
                       m_dotPaths.GetPositionTRef(),
                       GetVerticalLowColorMaps(),
                       [this](const size_t i) { return GetColorMixT(i); }}
{
}

Circle::Circle(Circle&&) noexcept = default;

Circle::~Circle() noexcept = default;

auto Circle::GetDotStartingPositions(const Point2dInt& centre, const float radius) noexcept
    -> std::vector<Point2dInt>
{
  auto positionT       = std::make_unique<TValue>(TValue::StepType::SINGLE_CYCLE, NUM_DOTS);
  const auto centrePos = Vec2dFlt{centre.ToFlt()};
  static constexpr auto DEFAULT_ANGLE_PARAMS = AngleParams{};
  const auto path =
      std::make_unique<CirclePath>(std::move(positionT), centrePos, radius, DEFAULT_ANGLE_PARAMS);

  auto dotStartingPositions = std::vector<Point2dInt>(NUM_DOTS);

  for (auto i = 0U; i < NUM_DOTS; ++i)
  {
    dotStartingPositions.at(i) = path->GetNextPoint();

    path->IncrementT();
  }

  return dotStartingPositions;
}

inline auto Circle::GetColorMixT([[maybe_unused]] const size_t colorIndex) const noexcept -> float
{
  return m_currentColorGridMixT;
}

auto Circle::GetVerticalMainColorMaps() const noexcept -> std::vector<const COLOR::IColorMap*>
{
  auto colorMaps = std::vector<const COLOR::IColorMap*>(NUM_DOTS);

  colorMaps.at(0) = &m_mainColorMaps->GetRandomColorMap();
  for (auto i = 1U; i < NUM_DOTS; ++i)
  {
    colorMaps.at(i) = colorMaps.at(0);
  }

  return colorMaps;
}

auto Circle::GetVerticalLowColorMaps() const noexcept -> std::vector<const COLOR::IColorMap*>
{
  auto colorMaps = std::vector<const COLOR::IColorMap*>(NUM_DOTS);

  colorMaps.at(0) = &m_lowColorMaps->GetRandomColorMap();
  for (auto i = 1U; i < NUM_DOTS; ++i)
  {
    colorMaps.at(i) = colorMaps.at(0);
  }

  return colorMaps;
}

auto Circle::SetWeightedColorMaps(
    const std::shared_ptr<const RandomColorMaps> weightedMainMaps,
    const std::shared_ptr<const RandomColorMaps> weightedLowMaps) noexcept -> void
{
  m_mainColorMaps = weightedMainMaps;
  m_lowColorMaps  = weightedLowMaps;

  const auto& newMainColorMap = m_mainColorMaps->GetRandomColorMap();
  m_mainColorMapsGrid.SetColorMaps(newMainColorMap, GetVerticalMainColorMaps());
  const auto& newLowColorMap = m_lowColorMaps->GetRandomColorMap();
  m_lowColorMapsGrid.SetColorMaps(newLowColorMap, GetVerticalLowColorMaps());
  m_currentColorGridMixT = m_goomRand.GetRandInRange(0.0F, 1.0F);

  m_linesMainColorMap                  = &newMainColorMap;
  m_linesLowColorMap                   = &newLowColorMap;
  static constexpr auto PROB_SHOW_LINE = 0.5F;
  m_showLine                           = m_goomRand.ProbabilityOf(PROB_SHOW_LINE);

  m_dotDiameters.ChangeDotDiameters();
  static constexpr auto PROB_ALTERNATE_MAIN_LOW_DOT_COLORS = 0.1F;
  m_alternateMainLowDotColors = m_goomRand.ProbabilityOf(PROB_ALTERNATE_MAIN_LOW_DOT_COLORS);
  m_dotDrawer->SetWeightedColorMaps(*weightedMainMaps);

  m_colorAdjustmentT.SetNumSteps(GetRandomNumColorAdjustmentSteps());
  m_minChromaFactor = m_goomRand.GetRandInRange(MIN_MIN_CHROMA_FACTOR, MAX_MIN_CHROMA_FACTOR);
  m_maxChromaFactor = m_goomRand.GetRandInRange(MIN_MAX_CHROMA_FACTOR, MAX_MAX_CHROMA_FACTOR);
}

auto Circle::SetBlankTime(const uint32_t blankTime) noexcept -> void
{
  if (m_blankTimer.Finished())
  {
    m_blankTimer.SetTimeLimit(blankTime, true);
  }
}

auto Circle::SetNewCircleCentreAndRadius(const Point2dInt& centre, const float radius) noexcept
    -> void
{
  m_dotPaths.SetDotStartingPositions(GetDotStartingPositions(centre, radius));
}

auto Circle::SetMovingTargetPoint(const Point2dInt& movingTargetPoint,
                                  float lerpTFromFixedTarget) noexcept -> void
{
  m_circleCentreTarget = lerp(m_circleCentreFixedTarget, movingTargetPoint, lerpTFromFixedTarget);
}

auto Circle::SetPathParams(const OscillatingFunction::Params& pathParams) noexcept -> void
{
  m_dotPaths.SetPathParams(pathParams);
}

auto Circle::Start() noexcept -> void
{
  m_updateNum = 0;
  m_blankTimer.SetToFinished();
  m_dotPaths.Reset();
}

auto Circle::UpdateAndDraw() noexcept -> void
{
  UpdateTime();
  UpdateColorAdjustment();
  DrawNextCircle();
}

inline auto Circle::UpdateTime() noexcept -> void
{
  ++m_updateNum;
  m_blankTimer.Increment();
}

inline auto Circle::UpdateColorAdjustment() noexcept -> void
{
  m_colorAdjustmentT.Increment();

  m_colorAdjustment.SetChromaFactor(
      STD20::lerp(m_minChromaFactor, m_maxChromaFactor, m_colorAdjustmentT()));
}

inline auto Circle::GetRandomNumColorAdjustmentSteps() const noexcept -> uint32_t
{
  return m_goomRand.GetRandInRange(MIN_NUM_COLOR_ADJUSTMENT_STEPS,
                                   MAX_NUM_COLOR_ADJUSTMENT_STEPS + 1);
}

inline auto Circle::DrawNextCircle() noexcept -> void
{
  if (not m_blankTimer.Finished())
  {
    return;
  }

  DrawNextCircleDots();
  ResetNumSteps();
  ResetCircleParams();
  IncrementTs();
}

inline auto Circle::IncrementTs() noexcept -> void
{
  m_dotPaths.IncrementPositionT();
  m_dotAttributeOffset = ModIncrement(m_dotAttributeOffset, NUM_DOTS);

  if ((not m_dotPaths.IsDelayed()) && m_dotPaths.HasPositionTJustHitEndBoundary())
  {
    m_blankTimer.ResetToZero();
  }
}

auto Circle::ResetNumSteps() noexcept -> void
{
  if (0 == m_newNumSteps)
  {
    return;
  }
  if (not HasPositionTJustHitABoundary())
  {
    return;
  }
  m_dotPaths.SetPositionTNumSteps(m_newNumSteps);
  m_newNumSteps = 0;
}

inline auto Circle::ResetCircleParams() noexcept -> void
{
  if (not HasPositionTJustHitABoundary())
  {
    return;
  }

  m_dotPaths.SetTarget(m_circleCentreTarget);
}

auto Circle::DrawNextCircleDots() noexcept -> void
{
  const auto nextDotPositions = m_dotPaths.GetNextDotPositions();

  const auto brightness     = GetCurrentBrightness();
  const auto dotBrightness  = GetDotBrightness(brightness);
  const auto lineBrightness = GetLineBrightness(brightness);
  const auto dotMainColors  = GetDotMainColors(dotBrightness);
  const auto dotLowColors   = GetDotLowColors(dotBrightness);

  auto iRotate         = m_dotAttributeOffset;
  auto tLineColor      = 0.0F;
  auto prevDotPosition = nextDotPositions[NUM_DOTS - 1];
  for (auto i = 0U; i < NUM_DOTS; ++i)
  {
    const auto dotPosition  = nextDotPositions.at(i);
    const auto dotDiameter  = m_dotDiameters.GetDotDiameters().at(iRotate);
    const auto dotMainColor = dotMainColors.at(iRotate);
    const auto dotLowColor  = dotLowColors.at(iRotate);

    m_dotDrawer->DrawDot(dotPosition, dotDiameter, dotMainColor, dotLowColor);
    DrawLine(prevDotPosition, dotPosition, lineBrightness, tLineColor);

    m_lastDrawnDots.at(i) = dotPosition;
    prevDotPosition       = dotPosition;
    tLineColor += T_LINE_COLOR_STEP;
    iRotate = ModIncrement(iRotate, NUM_DOTS);
  }
}

inline auto Circle::GetCurrentBrightness() const noexcept -> float
{
  static constexpr auto BRIGHTNESS_CUT = 0.001F;
  static constexpr auto T_NEAR_END     = 0.8F;
  return m_dotPaths.GetPositionT() < T_NEAR_END ? 1.0F : BRIGHTNESS_CUT;
}

inline auto Circle::GetDotBrightness(const float brightness) const noexcept -> float
{
  if (static constexpr auto T_NEAR_START = 0.1F; m_dotPaths.GetPositionT() > T_NEAR_START)
  {
    return brightness;
  }
  if (IsEven(m_updateNum))
  {
    static constexpr auto BRIGHTNESS_INCREASE = 10.0F;
    return BRIGHTNESS_INCREASE * brightness;
  }
  static constexpr auto BRIGHTNESS_INCREASE = 1.5F;
  return BRIGHTNESS_INCREASE * brightness;
}

inline auto Circle::IsSpecialUpdateNum() const noexcept -> bool
{
  if (static constexpr auto CLOSE_TO_START_T = 0.1F; m_dotPaths.GetPositionT() < CLOSE_TO_START_T)
  {
    return true;
  }
  static constexpr auto SPECIAL_UPDATE_MULTIPLE = 5U;
  return 0 == (m_updateNum % SPECIAL_UPDATE_MULTIPLE);
}

inline auto Circle::IsSpecialLineUpdateNum() const noexcept -> bool
{
  if (static constexpr auto CLOSE_TO_START_T = 0.1F; m_dotPaths.GetPositionT() < CLOSE_TO_START_T)
  {
    return true;
  }
  static constexpr auto LINE_UPDATE_MULTIPLE = 8U;
  return 0 == (m_updateNum % LINE_UPDATE_MULTIPLE);
}

inline auto Circle::GetLineBrightness(const float brightness) const noexcept -> float
{
  static constexpr auto BRIGHTNESS_INCREASE = 5.0F;
  return IsSpecialUpdateNum() ? (BRIGHTNESS_INCREASE * brightness) : brightness;
}

inline auto Circle::GetDotMainColors(const float dotBrightness) const noexcept -> std::vector<Pixel>
{
  auto dotColors = m_mainColorMapsGrid.GetNextColors();

  for (auto& color : dotColors)
  {
    color = GetFinalMainColor(dotBrightness, color);
  }

  return dotColors;
}

inline auto Circle::GetDotLowColors(const float dotBrightness) const noexcept -> std::vector<Pixel>
{
  static constexpr auto BRIGHTNESS_INCREASE = 1.1F;

  auto dotColors = m_lowColorMapsGrid.GetNextColors();

  for (auto& color : dotColors)
  {
    color = GetFinalMainColor(BRIGHTNESS_INCREASE * dotBrightness, color);
  }

  return dotColors;
}

inline auto Circle::GetFinalMainColor(const float brightness, const Pixel& mainColor) const noexcept
    -> Pixel
{
  static constexpr auto MAIN_COLOR_BRIGHTNESS = 1.0F;
  return GetCorrectedColor(brightness * MAIN_COLOR_BRIGHTNESS, mainColor);
}

inline auto Circle::GetFinalLowColor(const float brightness, const Pixel& lowColor) const noexcept
    -> Pixel
{
  static constexpr auto LOW_COLOR_BRIGHTNESS = 1.0F;
  return GetCorrectedColor(brightness * LOW_COLOR_BRIGHTNESS, lowColor);
}

inline auto Circle::GetCorrectedColor(const float brightness, const Pixel& color) const noexcept
    -> Pixel
{
  return m_colorAdjustment.GetAdjustment(brightness, color);
}

inline auto Circle::DrawLine(const Point2dInt& position1,
                             const Point2dInt& position2,
                             const float lineBrightness,
                             const float tLineColor) noexcept -> void
{
  const auto lastTDotColor = DrawLineDots(position1, position2, lineBrightness, tLineColor);
  DrawConnectingLine(position1, position2, lineBrightness, lastTDotColor);
}

inline auto Circle::DrawLineDots(const Point2dInt& position1,
                                 const Point2dInt& position2,
                                 const float lineBrightness,
                                 const float tLineColor) noexcept -> float
{
  static constexpr auto NUM_LINE_DOTS = 5U;

  static constexpr auto T_DOT_POS_STEP = 1.0F / static_cast<float>(NUM_LINE_DOTS);
  auto tDotPos                         = T_DOT_POS_STEP;

  static constexpr auto T_DOT_COLOR_STEP = T_LINE_COLOR_STEP / static_cast<float>(NUM_LINE_DOTS);

  auto tDotColor = tLineColor + T_DOT_COLOR_STEP;
  for (auto i = 0U; i < (NUM_LINE_DOTS - 1); ++i)
  {
    const auto dotPos = lerp(position1, position2, tDotPos);
    const auto mainColor =
        GetFinalMainColor(lineBrightness, m_linesMainColorMap->GetColor(tDotColor));
    const auto lowColor = GetFinalLowColor(lineBrightness, m_linesLowColorMap->GetColor(tDotColor));

    DrawDot(i, dotPos, mainColor, lowColor);

    tDotPos += T_DOT_POS_STEP;
    tDotColor += T_DOT_COLOR_STEP;
  }

  return tDotColor;
}

inline auto Circle::DrawDot(const uint32_t dotNum,
                            const Point2dInt& pos,
                            const Pixel& mainColor,
                            const Pixel& lowColor) noexcept -> void
{
  if (m_alternateMainLowDotColors and UTILS::MATH::IsEven(dotNum))
  {
    m_dotDrawer->DrawDot(pos, m_helper.lineDotDiameter, lowColor, mainColor);
  }
  else
  {
    m_dotDrawer->DrawDot(pos, m_helper.lineDotDiameter, mainColor, lowColor);
  }
}

auto Circle::DrawConnectingLine(const Point2dInt& position1,
                                const Point2dInt& position2,
                                const float lineBrightness,
                                const float tDotColor) noexcept -> void
{
  if ((not m_showLine) or (not IsSpecialLineUpdateNum()))
  {
    return;
  }

  static constexpr auto LINE_THICKNESS = 1U;
  const auto mainColor =
      GetFinalMainColor(lineBrightness, m_linesMainColorMap->GetColor(tDotColor));
  const auto lowColor = GetFinalLowColor(lineBrightness, m_linesLowColorMap->GetColor(tDotColor));

  m_draw.Line(position1, position2, {mainColor, lowColor}, LINE_THICKNESS);
}

Circle::DotDrawer::DotDrawer(DRAW::IGoomDraw& draw,
                             const IGoomRand& goomRand,
                             const Circle::Helper& helper) noexcept
  : m_draw{draw},
    m_goomRand{goomRand},
    m_helper{helper},
    m_bgndMainColorMixT{m_goomRand.GetRandInRange(MIN_BGND_MIX_T, MAX_BGND_MIX_T)},
    m_bgndLowColorMixT{m_goomRand.GetRandInRange(MIN_BGND_MIX_T, MAX_BGND_MIX_T)},
    m_decorationType{GetRandomDecorationType()},
    m_differentColor{GetRandomDifferentColor(RandomColorMaps{m_goomRand})}
{
}

inline auto Circle::DotDrawer::SetWeightedColorMaps(const RandomColorMaps& weightedMaps) noexcept
    -> void
{
  m_bgndMainColorMixT      = m_goomRand.GetRandInRange(MIN_BGND_MIX_T, MAX_BGND_MIX_T);
  m_bgndLowColorMixT       = m_goomRand.GetRandInRange(MIN_BGND_MIX_T, MAX_BGND_MIX_T);
  m_decorationType         = GetRandomDecorationType();
  m_differentColor         = GetRandomDifferentColor(weightedMaps);
  m_outerCircleDotColorMap = &weightedMaps.GetRandomColorMap();

  static constexpr auto PROB_CIRCLES = 0.5F;
  m_doCircleDotShapes                = m_goomRand.ProbabilityOf(PROB_CIRCLES);
  m_outerCircleDotColorMix =
      m_goomRand.GetRandInRange(MIN_OUTER_CIRCLE_DOT_COLOR_MIX_T, MAX_OUTER_CIRCLE_DOT_COLOR_MIX_T);
}

inline auto Circle::DotDrawer::DrawDot(const Point2dInt& pos,
                                       const uint32_t diameter,
                                       const Pixel& mainColor,
                                       const Pixel& lowColor) noexcept -> void
{
  if (m_doCircleDotShapes)
  {
    DrawCircleDot(pos, diameter, mainColor, lowColor, *m_outerCircleDotColorMap);
  }
  else
  {
    DrawBitmapDot(pos, diameter, mainColor, lowColor);
  }
}

auto Circle::DotDrawer::DrawCircleDot(const Point2dInt& centre,
                                      const uint32_t diameter,
                                      const Pixel& mainColor,
                                      const Pixel& lowColor,
                                      const IColorMap& innerColorMap) noexcept -> void
{
  const auto maxRadius = static_cast<int32_t>(diameter + 3) / 2;
  auto innerColorT =
      TValue{UTILS::TValue::StepType::SINGLE_CYCLE, static_cast<uint32_t>(maxRadius - 1)};
  static constexpr auto INNER_COLOR_CUTOFF_RADIUS = 4;

  static constexpr auto MIN_BRIGHTNESS = 1.0F;
  static constexpr auto MAX_BRIGHTNESS = 10.0F;
  auto brightnessT = TValue{TValue::StepType::SINGLE_CYCLE, static_cast<uint32_t>(maxRadius)};

  // '> 1' means leave a little hole in the middle of the circles.
  for (int32_t radius = maxRadius; radius > 1; --radius)
  {
    const auto brightness = STD20::lerp(MIN_BRIGHTNESS, MAX_BRIGHTNESS, brightnessT());
    const auto innerColor = innerColorMap.GetColor(innerColorT());
    const auto colors =
        radius <= INNER_COLOR_CUTOFF_RADIUS
            ? GetCircleColors(brightness, mainColor, lowColor)
            : GetCircleColorsWithInner(
                  brightness, mainColor, lowColor, innerColor, m_outerCircleDotColorMix);

    m_draw.Circle(centre, radius, colors);

    brightnessT.Increment();
    innerColorT.Increment();
  }
}

inline auto Circle::DotDrawer::GetCircleColors(const float brightness,
                                               const Pixel& mainColor,
                                               const Pixel& lowColor) noexcept -> MultiplePixels
{
  return {
      GetBrighterColor(CIRCLE_DOT_MAIN_COLOR_BRIGHTNESS_FACTOR * brightness, mainColor),
      GetBrighterColor(CIRCLE_DOT_LOW_COLOR_BRIGHTNESS_FACTOR * brightness, lowColor),
  };
}

inline auto Circle::DotDrawer::GetCircleColorsWithInner(const float brightness,
                                                        const Pixel& mainColor,
                                                        const Pixel& lowColor,
                                                        const Pixel& innerColor,
                                                        const float innerColorMix) noexcept
    -> MultiplePixels
{
  return {
      GetBrighterColor(CIRCLE_DOT_MAIN_COLOR_BRIGHTNESS_FACTOR * brightness,
                       IColorMap::GetColorMix(mainColor, innerColor, innerColorMix)),
      GetBrighterColor(CIRCLE_DOT_LOW_COLOR_BRIGHTNESS_FACTOR * brightness,
                       IColorMap::GetColorMix(lowColor, innerColor, innerColorMix)),
  };
}

auto Circle::DotDrawer::DrawBitmapDot(const Point2dInt& position,
                                      const uint32_t diameter,
                                      const Pixel& mainColor,
                                      const Pixel& lowColor) noexcept -> void
{
  const auto getMainColor =
      [this, &mainColor, &diameter](const size_t x, const size_t y, const Pixel& bgnd)
  {
    static constexpr auto MAIN_COLOR_BRIGHTNESS = 1.0F;
    const auto color = GetBrighterColor(MAIN_COLOR_BRIGHTNESS, mainColor);
    return GetDotMixedColor(x, y, diameter, bgnd, color, m_bgndMainColorMixT);
  };

  const auto getLowColor =
      [this, &lowColor, &diameter](const size_t x, const size_t y, const Pixel& bgnd)
  {
    static constexpr auto LOW_COLOR_BRIGHTNESS = 2.0F;
    const auto color                           = GetBrighterColor(LOW_COLOR_BRIGHTNESS, lowColor);
    return GetDotMixedColor(x, y, diameter, bgnd, color, m_bgndLowColorMixT);
  };

  m_draw.Bitmap(position, m_helper.bitmapGetter.GetBitmap(diameter), {getMainColor, getLowColor});
}

inline auto Circle::DotDrawer::GetRandomDecorationType() const noexcept -> DecorationType
{
  return static_cast<DecorationType>(m_goomRand.GetRandInRange(0U, NUM<DecorationType>));
}

inline auto Circle::DotDrawer::GetRandomDifferentColor(
    const RandomColorMaps& weightedMaps) const noexcept -> Pixel
{
  return weightedMaps.GetRandomColorMap().GetColor(m_goomRand.GetRandInRange(0.0F, 1.0F));
}

inline auto Circle::DotDrawer::GetDotMixedColor(const size_t x,
                                                const size_t y,
                                                const uint32_t diameter,
                                                const Pixel& bgnd,
                                                const Pixel& color,
                                                const float mixT) const noexcept -> Pixel
{
  if (0 == bgnd.A())
  {
    return BLACK_PIXEL;
  }

  const auto mixedColor = IColorMap::GetColorMix(bgnd, color, mixT);

  if (!IsSpecialPoint(x, y, diameter))
  {
    return mixedColor;
  }

  static constexpr auto DIFFERENT_COLOR_BRIGHTNESS = 2.0F;
  static constexpr auto SPECIAL_BRIGHTNESS         = 2.0F;

  switch (m_decorationType)
  {
    case DecorationType::NO_DECORATION:
      return mixedColor;
    case DecorationType::BLACK_LINES:
      return BLACK_PIXEL;
    case DecorationType::DIFFERENT_COLORS:
      return GetBrighterColor(DIFFERENT_COLOR_BRIGHTNESS, m_differentColor);
    case DecorationType::BRIGHT_LINES:
      return GetBrighterColor(SPECIAL_BRIGHTNESS, mixedColor);
    default:
      FailFast();
      return BLACK_PIXEL;
  }
}

inline auto Circle::DotDrawer::IsSpecialPoint(const size_t x,
                                              const size_t y,
                                              const uint32_t diameter) noexcept -> bool
{
  if (static constexpr auto EDGE_CUTOFF = 3U; (x <= EDGE_CUTOFF) ||
                                              (x >= (diameter - EDGE_CUTOFF)) ||
                                              (y <= EDGE_CUTOFF) || (y >= (diameter - EDGE_CUTOFF)))
  {
    return false;
  }
  if (((U_HALF * diameter) == x) || ((U_HALF * diameter) == y))
  {
    return true;
  }
  if ((x == y) || ((diameter - x) == y))
  {
    return true;
  }

  return false;
}

} // namespace GOOM::VISUAL_FX::CIRCLES
