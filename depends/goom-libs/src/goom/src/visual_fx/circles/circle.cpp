#include "circle.h"

#include "color/colormaps.h"
#include "color/colorutils.h"
#include "color/random_colormaps.h"
#include "draw/goom_draw.h"
#include "goom_graphic.h"
#include "point2d.h"
#include "utils/enumutils.h"
#include "utils/math/goom_rand_base.h"
#include "utils/math/misc.h"
#include "utils/math/paths.h"
#include "utils/t_values.h"
#include "utils/timer.h"

#include <array>
#undef NDEBUG
#include <cassert>
#include <cmath>
#include <cstdint>
#include <memory>
#include <vector>

namespace GOOM::VISUAL_FX::CIRCLES
{

using COLOR::GetAllSlimMaps;
using COLOR::GetIncreasedChroma;
using COLOR::IColorMap;
using COLOR::RandomColorMaps;
using DRAW::IGoomDraw;
using UTILS::NUM;
using UTILS::Timer;
using UTILS::TValue;
using UTILS::MATH::IGoomRand;
using UTILS::MATH::IsEven;
using UTILS::MATH::PathParams;

Circle::Circle(const FxHelper& fxHelper,
               const Helper& helper,
               const Params& circleParams,
               const PathParams& pathParams)
  : m_draw{fxHelper.GetDraw()},
    m_goomInfo{fxHelper.GetGoomInfo()},
    m_goomRand{fxHelper.GetGoomRand()},
    m_helper{helper},
    m_circleCentreTarget{circleParams.circleCentreTarget},
    m_dotPaths{NUM_DOTS, m_circleCentreTarget, circleParams.circleRadius, pathParams},
    m_dotDiameters{m_goomRand, NUM_DOTS, m_helper.minDotDiameter, m_helper.maxDotDiameter},
    m_colorTs{GetInitialColorTs()},
    m_colorMaps{GetAllSlimMaps(m_goomRand)},
    m_lowColorMaps{m_colorMaps}
{
}

void Circle::SetWeightedColorMaps(const std::shared_ptr<RandomColorMaps> weightedMaps,
                                  const std::shared_ptr<RandomColorMaps> weightedLowMaps)
{
  m_colorMaps = weightedMaps;
  m_lowColorMaps = weightedLowMaps;

  ChangeDotColorMaps();
  m_dotDiameters.ChangeDotDiameters();
}

void Circle::SetZoomMidpoint([[maybe_unused]] const Point2dInt& zoomMidpoint)
{
  // Don't need the zoom midpoint.
}

void Circle::SetPathParams(const PathParams& pathParams)
{
  m_dotPaths.SetPathParams(pathParams);
}

void Circle::Start()
{
  ChangeDotColorMaps();
}

void Circle::UpdateAndDraw()
{
  UpdateTime();
  UpdateSpeeds();
  DrawNextCircle();
}

auto Circle::GetInitialColorTs() -> std::vector<TValue>
{
  std::vector<TValue> colorTs{};
  for (size_t i = 0; i < NUM_DOTS; ++i)
  {
    colorTs.emplace_back(TValue::StepType::CONTINUOUS_REVERSIBLE, NUM_COLOR_STEPS, 0.0F);
  }
  return colorTs;
}

void Circle::ChangeDotColorMaps()
{
  m_dotColorMaps.at(0) = &m_colorMaps->GetRandomColorMap();
  m_dotLowColorMaps.at(0) = &m_lowColorMaps->GetRandomColorMap();

  for (size_t i = 1; i < NUM_DOTS; ++i)
  {
    m_dotColorMaps.at(i) = m_dotColorMaps.at(0);
    m_dotLowColorMaps.at(i) = m_dotLowColorMaps.at(0);
  }

  m_linesColorMap = m_dotColorMaps.at(0);
  m_linesLowColorMap = m_dotLowColorMaps.at(0);
}

inline void Circle::UpdateTime()
{
  ++m_updateNum;
  m_blankTimer.Increment();
}

inline void Circle::UpdateSpeeds()
{
  UpdatePositionSpeed();
  UpdateColorLerpSpeed();
}

inline void Circle::UpdatePositionSpeed()
{
  if (constexpr float PROB_NO_SPEED_CHANGE = 0.7F; m_goomRand.ProbabilityOf(PROB_NO_SPEED_CHANGE))
  {
    return;
  }

  m_dotPaths.SetPositionTNumSteps(GetPositionTNumSteps());
}

inline auto Circle::GetPositionTNumSteps() const -> uint32_t
{
  return std::min(MIN_POSITION_STEPS + m_goomInfo.GetSoundInfo().GetTimeSinceLastGoom(),
                  MAX_POSITION_STEPS);
}

void Circle::UpdateColorLerpSpeed()
{
  if (constexpr float PROB_NO_SPEED_CHANGE = 0.3F; m_goomRand.ProbabilityOf(PROB_NO_SPEED_CHANGE))
  {
    return;
  }

  constexpr uint32_t MAX_COLOR_STEPS_DIVISOR = 10;

  if (constexpr float PROB_SAME_T_S = 0.9F; m_goomRand.ProbabilityOf(PROB_SAME_T_S))
  {
    const uint32_t colorStepsDivisor = m_goomRand.GetRandInRange(1U, MAX_COLOR_STEPS_DIVISOR);
    for (auto& colorT : m_colorTs)
    {
      colorT.SetNumSteps(GetColorTNumSteps(colorStepsDivisor));
    }
    return;
  }

  for (auto& colorT : m_colorTs)
  {
    const uint32_t colorStepsDivisor = m_goomRand.GetRandInRange(1U, MAX_COLOR_STEPS_DIVISOR);
    colorT.SetNumSteps(GetColorTNumSteps(colorStepsDivisor));
  }
}

inline auto Circle::GetColorTNumSteps(const uint32_t colorStepsDivisor) const -> uint32_t
{
  return std::max(m_dotPaths.GetPositionTNumSteps() / colorStepsDivisor, colorStepsDivisor);
}

inline void Circle::DrawNextCircle()
{
  if (!m_blankTimer.Finished())
  {
    return;
  }

  DrawNextCircleDots();
  ResetCircleParams();
  IncrementTs();
}

inline void Circle::IncrementTs()
{
  if (m_dotPaths.HasPositionTJustHitStartBoundary())
  {
    m_blankTimer.ResetToZero();
  }

  m_dotPaths.IncrementPositionT();

  for (auto& colorT : m_colorTs)
  {
    colorT.Increment();
  }
}

void Circle::DrawNextCircleDots()
{
  const std::vector<Point2dInt> nextDotPositions = m_dotPaths.GetNextDotPositions();

  const float brightness = GetCurrentBrightness();
  const float dotBrightness = GetDotBrightness(brightness);
  const float lineBrightness = GetLineBrightness(brightness);

  float tLineColor = 0.0F;
  Point2dInt prevDotPosition = nextDotPositions[NUM_DOTS - 1];
  for (size_t i = 0; i < NUM_DOTS; ++i)
  {
    const Point2dInt dotPosition = nextDotPositions.at(i);
    const uint32_t dotDiameter = m_dotDiameters.GetDotDiameters().at(i);
    const Pixel dotColor = GetDotColor(i, dotBrightness);
    const Pixel dotLowColor = GetDotLowColor(i, dotBrightness);

    DrawDot(dotPosition, dotDiameter, dotColor, dotLowColor);
    DrawLine(prevDotPosition, dotPosition, lineBrightness, tLineColor);

    prevDotPosition = dotPosition;
    tLineColor += T_LINE_COLOR_STEP;
  }
}

inline auto Circle::GetCurrentBrightness() const -> float
{
  constexpr float BRIGHTNESS_CUT = 0.001F;
  constexpr float INNER_POSITION_MARGIN = 0.05F;
  return m_dotPaths.GetPositionT() < (1.0F - INNER_POSITION_MARGIN) ? 1.0F : BRIGHTNESS_CUT;
}

inline auto Circle::GetDotBrightness(const float brightness) const -> float
{
  constexpr float T_NEAR_START = 0.1F;
  constexpr float BRIGHTNESS_INCREASE = 5.0F;
  return (m_dotPaths.GetPositionT() < T_NEAR_START) && IsEven(m_updateNum)
             ? (BRIGHTNESS_INCREASE * brightness)
             : brightness;
}

inline auto Circle::IsSpecialUpdateNum() const -> bool
{
  constexpr uint64_t SPECIAL_UPDATE_MULTIPLE = 5;
  return 0 == (m_updateNum % SPECIAL_UPDATE_MULTIPLE);
}

inline auto Circle::GetLineBrightness(const float brightness) const -> float
{
  constexpr float BRIGHTNESS_INCREASE = 10.0F;
  return IsSpecialUpdateNum() ? (BRIGHTNESS_INCREASE * brightness) : brightness;
}

inline auto Circle::GetDotColor(const size_t i, const float dotBrightness) const -> Pixel
{
  const float tColor = m_colorTs.at(i)();
  const Pixel colorToUse = IsSpecialUpdateNum() ? m_dotLowColorMaps.at(i)->GetColor(tColor)
                                                : m_dotColorMaps.at(i)->GetColor(tColor);
  return GetFinalColor(dotBrightness, colorToUse);
}

inline auto Circle::GetDotLowColor(const size_t i, const float dotBrightness) const -> Pixel
{
  const float tColor = m_colorTs.at(i)();
  constexpr float BRIGHTNESS_INCREASE = 1.1F;
  return GetFinalLowColor(BRIGHTNESS_INCREASE * dotBrightness,
                          m_dotLowColorMaps.at(i)->GetColor(tColor));
}

inline auto Circle::GetRandomCircleCentreTargetPosition() const -> Point2dInt
{
  constexpr int32_t MARGIN = 50;
  const Point2dInt randomPosition{
      m_goomRand.GetRandInRange(MARGIN, static_cast<int32_t>(m_draw.GetScreenWidth() - MARGIN)),
      m_goomRand.GetRandInRange(MARGIN, static_cast<int32_t>(m_draw.GetScreenHeight()) - MARGIN)};

  constexpr float TARGET_T = 0.1F;
  return lerp(m_circleCentreTarget, randomPosition, TARGET_T);
}

inline void Circle::ResetCircleParams()
{
  if ((!m_dotPaths.HasPositionTJustHitStartBoundary()) &&
      (!m_dotPaths.HasPositionTJustHitEndBoundary()))
  {
    return;
  }

  m_dotPaths.SetTarget(GetRandomCircleCentreTargetPosition());
}

inline auto Circle::GetFinalColor(const float brightness, const Pixel& color) const -> Pixel
{
  constexpr float COLOR_BRIGHTNESS = 15.0F;
  return GetCorrectedColor(brightness * COLOR_BRIGHTNESS, color);
}

inline auto Circle::GetFinalLowColor(const float brightness, const Pixel& lowColor) const -> Pixel
{
  constexpr float LOW_COLOR_BRIGHTNESS = 15.0F;
  return GetCorrectedColor(brightness * LOW_COLOR_BRIGHTNESS, lowColor);
}

inline auto Circle::GetCorrectedColor(const float brightness, const Pixel& color) const -> Pixel
{
  return m_helper.gammaCorrect.GetCorrection(brightness, GetIncreasedChroma(color));
}

inline void Circle::DrawDot(const Point2dInt& pos,
                            const uint32_t diameter,
                            const Pixel& color,
                            const Pixel& lowColor)
{
  const auto getColor1 =
      [&color]([[maybe_unused]] const size_t x, [[maybe_unused]] const size_t y, const Pixel& bgnd)
  {
    if (0 == bgnd.A())
    {
      return Pixel::BLACK;
    }
    constexpr float BGND_COLOR_T_MIX = 0.3F;
    return IColorMap::GetColorMix(bgnd, color, BGND_COLOR_T_MIX);
  };
  const auto getColor2 = [&lowColor]([[maybe_unused]] const size_t x,
                                     [[maybe_unused]] const size_t y, const Pixel& bgnd)
  {
    if (0 == bgnd.A())
    {
      return Pixel::BLACK;
    }
    constexpr float BGND_LOW_COLOR_T_MIX = 0.3F;
    return IColorMap::GetColorMix(bgnd, lowColor, BGND_LOW_COLOR_T_MIX);
  };

  m_draw.Bitmap(pos.x, pos.y, m_helper.bitmapGetter.GetBitmap(diameter), {getColor1, getColor2});
}

inline void Circle::DrawLine(const Point2dInt& pos1,
                             const Point2dInt& pos2,
                             const float lineBrightness,
                             const float tLineColor)
{
  constexpr uint32_t NUM_LINE_DOTS = 5;

  constexpr float T_DOT_POS_STEP = 1.0F / static_cast<float>(NUM_LINE_DOTS);
  float tDotPos = T_DOT_POS_STEP;

  constexpr float T_DOT_COLOR_STEP = T_LINE_COLOR_STEP / static_cast<float>(NUM_LINE_DOTS);

  float tDotColor = tLineColor + T_DOT_COLOR_STEP;
  for (uint32_t i = 0; i < (NUM_LINE_DOTS - 1); ++i)
  {
    const Point2dInt dotPos = lerp(pos1, pos2, tDotPos);
    DrawDot(dotPos, m_helper.minDotDiameter,
            GetFinalColor(lineBrightness, m_linesColorMap->GetColor(tDotColor)),
            GetFinalLowColor(lineBrightness, m_linesLowColorMap->GetColor(tDotColor)));
    tDotPos += T_DOT_POS_STEP;
    tDotColor += T_DOT_COLOR_STEP;
  }
  //m_draw.Line(pos1.x, pos1.y, pos2.x, pos2.y, {color, lowColor}, lineThickness);
}

} // namespace GOOM::VISUAL_FX::CIRCLES
