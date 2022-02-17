#include "circle.h"

#include "color/colormaps.h"
#include "color/colorutils.h"
#include "color/random_colormaps.h"
#include "draw/goom_draw.h"
#include "goom_graphic.h"
#include "point2d.h"
#include "utils/enumutils.h"
#include "utils/math/goom_rand_base.h"
#include "utils/math/mathutils.h"
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
using UTILS::MATH::GetHalf;
using UTILS::MATH::IGoomRand;
using UTILS::MATH::IsEven;
using UTILS::MATH::m_two_pi;

Circle::Circle(const FxHelper& fxHelper, const Helper& helper, const Params& circleParams)
  : m_draw{fxHelper.GetDraw()},
    m_goomInfo{fxHelper.GetGoomInfo()},
    m_goomRand{fxHelper.GetGoomRand()},
    m_helper{helper},
    m_circleCentreTarget{circleParams.circleCentreTarget},
    m_colorMaps{GetAllSlimMaps(m_goomRand)},
    m_currentCircleCentreTarget{GetRandomCircleCentreTargetPosition()},
    m_dotDiameters{GetInitialDotDiameters(m_helper.maxDotDiameter)},
    m_dotStartingPositions{
        GetStartingDotPositions(m_goomRand,
                                {static_cast<int32_t>(GetHalf(m_draw.GetScreenWidth())),
                                 static_cast<int32_t>(GetHalf(m_draw.GetScreenHeight()))},
                                circleParams.circleRadius)},
    m_colorTs{GetInitialColorTs()}
{
}

void Circle::SetWeightedColorMaps(const std::shared_ptr<RandomColorMaps> weightedMaps,
                                  const std::shared_ptr<RandomColorMaps> weightedLowMaps)
{
  m_colorMaps = weightedMaps;
  m_lowColorMaps = weightedLowMaps;

  ChangeDotColorMaps();
  ChangeDotDiameters();
}

void Circle::SetZoomMidpoint([[maybe_unused]] const Point2dInt& zoomMidpoint)
{
  // Don't need the zoom midpoint.
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

auto Circle::GetInitialDotDiameters(const uint32_t maxDotDiameter)
    -> std::array<uint32_t, NUM_DOT_PATHS>
{
  std::array<uint32_t, NUM_DOT_PATHS> diameters{};
  diameters.fill(maxDotDiameter);
  return diameters;
}

auto Circle::GetInitialColorTs() -> std::vector<TValue>
{
  std::vector<TValue> colorTs{};
  for (size_t i = 0; i < NUM_DOT_PATHS; ++i)
  {
    colorTs.emplace_back(TValue::StepType::CONTINUOUS_REVERSIBLE, NUM_COLOR_STEPS, 0.0F);
  }
  return colorTs;
}

auto Circle::GetStartingDotPositions([[maybe_unused]] const IGoomRand& goomRand,
                                     const Point2dInt& centre,
                                     const float radius) -> std::array<Point2dInt, NUM_DOT_PATHS>
{
  std::array<Point2dInt, NUM_DOT_PATHS> startingDotPositions{};
  const float angleStep = UTILS::MATH::m_two_pi / static_cast<float>(NUM_DOT_PATHS);
  float angle = 0.0F;
  for (size_t i = 0; i < NUM_DOT_PATHS; ++i)
  {
    const auto x = static_cast<int32_t>(std::lround(radius * std::cos(angle)));
    const auto y = static_cast<int32_t>(std::lround(radius * std::sin(angle)));

    startingDotPositions.at(i) = centre + Vec2dInt{x, y};

    angle += angleStep;
  }

  return startingDotPositions;
}

void Circle::ChangeDotColorMaps()
{
  m_dotColorMaps.at(0) = &GetRandomColorMap();
  m_dotLowColorMaps.at(0) = &GetRandomLowColorMap();

  for (size_t i = 1; i < NUM_DOT_PATHS; ++i)
  {
    m_dotColorMaps.at(i) = m_dotColorMaps.at(0);
    m_dotLowColorMaps.at(i) = m_dotLowColorMaps.at(0);
  }

  m_linesColorMap = m_dotColorMaps.at(0);
  m_linesLowColorMap = m_dotLowColorMaps.at(0);
}

void Circle::ChangeDotDiameters()
{
  const uint32_t fixedDotDiameter =
      m_goomRand.GetRandInRange(m_helper.minDotDiameter, m_helper.maxDotDiameter + 1);

  for (auto& dotDiameter : m_dotDiameters)
  {
    dotDiameter = fixedDotDiameter;
  }
}

inline auto Circle::GetRandomColorMap() const -> const IColorMap&
{
  assert(m_colorMaps);
  return m_colorMaps->GetRandomColorMap(m_colorMaps->GetRandomGroup());
}

inline auto Circle::GetRandomLowColorMap() const -> const IColorMap&
{
  assert(m_lowColorMaps);
  return m_lowColorMaps->GetRandomColorMap(m_lowColorMaps->GetRandomGroup());
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

  const uint32_t numSteps = std::min(
      MIN_POSITION_STEPS + m_goomInfo.GetSoundInfo().GetTimeSinceLastGoom(), MAX_POSITION_STEPS);
  m_positionT.SetNumSteps(numSteps);
}

void Circle::UpdateColorLerpSpeed()
{
  if (constexpr float PROB_COLOR_CHANGE = 0.3F; m_goomRand.ProbabilityOf(PROB_COLOR_CHANGE))
  {
    return;
  }

  constexpr uint32_t MAX_COLOR_STEPS_DIVISOR = 10;

  if (constexpr float PROB_SAME_TS = 0.9F; m_goomRand.ProbabilityOf(PROB_SAME_TS))
  {
    const uint32_t colorStepsDivisor = m_goomRand.GetRandInRange(1U, MAX_COLOR_STEPS_DIVISOR);
    for (auto& colorT : m_colorTs)
    {
      colorT.SetNumSteps(
          std::max(m_positionT.GetNumSteps() / colorStepsDivisor, colorStepsDivisor));
    }
    return;
  }

  for (auto& colorT : m_colorTs)
  {
    const uint32_t colorStepsDivisor = m_goomRand.GetRandInRange(1U, MAX_COLOR_STEPS_DIVISOR);
    colorT.SetNumSteps(std::max(m_positionT.GetNumSteps() / colorStepsDivisor, colorStepsDivisor));
  }
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
  if (m_positionT.HasJustHitStartBoundary())
  {
    m_blankTimer.ResetToZero();
  }

  m_positionT.Increment();

  for (auto& colorT : m_colorTs)
  {
    colorT.Increment();
  }
}

void Circle::DrawNextCircleDots()
{
  constexpr uint32_t NUM_DOTS = NUM_DOT_PATHS;
  const std::array<Point2dInt, NUM_DOTS> nextDotPositions = GetNextDotPositions();

  const uint32_t fixedDotDiameter =
      m_goomRand.GetRandInRange(m_helper.minDotDiameter + 2, m_helper.maxDotDiameter + 1);

  const float brightness = GetCurrentBrightness();
  const float dotBrightness = GetDotBrightness(brightness);
  const float lineBrightness = GetLineBrightness(brightness);

  float tLineColor = 0.0F;
  Point2dInt prevDotPosition = nextDotPositions[NUM_DOTS - 1];
  for (size_t i = 0; i < NUM_DOTS; ++i)
  {
    const Point2dInt dotPosition = nextDotPositions.at(i);

    DrawDot(dotPosition, fixedDotDiameter, GetDotColor(i, dotBrightness),
            GetDotLowColor(i, dotBrightness));
    DrawLine(prevDotPosition, dotPosition, lineBrightness, tLineColor);

    prevDotPosition = dotPosition;
    tLineColor += T_LINE_COLOR_STEP;
  }
}

inline auto Circle::GetCurrentBrightness() const -> float
{
  constexpr float BRIGHTNESS_CUT = 0.001F;
  return m_positionT() < (1.0F - INNER_POSITION_MARGIN) ? 1.0F : BRIGHTNESS_CUT;
}

inline auto Circle::GetDotBrightness(const float brightness) const -> float
{
  constexpr float T_NEAR_START = 0.1F;
  constexpr float BRIGHTNESS_INCREASE = 5.0F;
  return (m_positionT() < T_NEAR_START) && IsEven(m_updateNum) ? (BRIGHTNESS_INCREASE * brightness)
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

auto Circle::GetNextDotPositions() const -> std::array<Point2dInt, NUM_DOT_PATHS>
{
  const float tPosition = m_positionT() < (1.0F - INNER_POSITION_MARGIN)
                              ? m_positionT()
                              : (1.0F - INNER_POSITION_MARGIN);

  const Point2dInt dotOffset = GetNextDotOffset(tPosition);

  std::array<Point2dInt, NUM_DOT_PATHS> nextDotPositions{};
  for (size_t i = 0; i < NUM_DOT_PATHS; ++i)
  {
    //const Point2dInt jitter = {m_goomRand.GetRandInRange(-2, +3),
    //                           m_goomRand.GetRandInRange(-2, +3)};
    nextDotPositions.at(i) = dotOffset + Vec2dInt{lerp(m_dotStartingPositions.at(i),
                                                       m_currentCircleCentreTarget, tPosition)};
  }
  return nextDotPositions;
}

inline void Circle::ChangeDotOffset()
{
  m_dotOffsetType = static_cast<DotOffsetType>(m_goomRand.GetRandInRange(0U, NUM<DotOffsetType>));
}

inline auto Circle::GetNextDotOffset(const float t) const -> Point2dInt
{
  constexpr float AMP_X = 100.0F;
  constexpr float AMP_Y = 100.0F;
  constexpr float FREQ_X = 1.0F * m_two_pi;
  constexpr float FREQ_Y = 2.0F * m_two_pi;

  switch (m_dotOffsetType)
  {
    case DotOffsetType::SIN_OFFSET:
      return Point2dFlt{AMP_X * std::sin(FREQ_X * t), AMP_Y * std::sin(FREQ_Y * t)}.ToInt();
    case DotOffsetType::COS_OFFSET:
      return Point2dFlt{AMP_X * std::cos(FREQ_X * t), AMP_Y * std::cos(FREQ_Y * t)}.ToInt();
    case DotOffsetType::SIN_COS_OFFSET:
      return Point2dFlt{AMP_X * std::sin(FREQ_X * t), AMP_Y * std::cos(FREQ_Y * t)}.ToInt();
    case DotOffsetType::COS_SIN_OFFSET:
      return Point2dFlt{AMP_X * std::cos(FREQ_X * t), AMP_Y * std::sin(FREQ_Y * t)}.ToInt();
    default:
      throw std::logic_error("Invalid DotOffsetType enum.");
  }
}

auto Circle::GetRandomCircleCentreTargetPosition() const -> Point2dInt
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
  if ((!m_positionT.HasJustHitStartBoundary()) && (!m_positionT.HasJustHitEndBoundary()))
  {
    return;
  }

  m_currentCircleCentreTarget = GetRandomCircleCentreTargetPosition();

  ChangeDotOffset();
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
  constexpr uint32_t NUM_DOTS = 5;
  constexpr float T_STEP = 1.0F / static_cast<float>(NUM_DOTS);
  float t = T_STEP;
  constexpr float T_LINE_STEP = T_LINE_COLOR_STEP / static_cast<float>(NUM_DOTS);
  float tLine = tLineColor + T_LINE_STEP;
  for (uint32_t i = 0; i < (NUM_DOTS - 1); ++i)
  {
    const Point2dInt pos = lerp(pos1, pos2, t);
    constexpr uint32_t DOT_DIAMETER = 5;
    DrawDot(pos, DOT_DIAMETER, GetFinalColor(lineBrightness, m_linesColorMap->GetColor(tLine)),
            GetFinalLowColor(lineBrightness, m_linesLowColorMap->GetColor(tLine)));
    t += T_STEP;
    tLine += T_LINE_STEP;
  }
  //m_draw.Line(pos1.x, pos1.y, pos2.x, pos2.y, {color, lowColor}, lineThickness);
}

} // namespace GOOM::VISUAL_FX::CIRCLES
