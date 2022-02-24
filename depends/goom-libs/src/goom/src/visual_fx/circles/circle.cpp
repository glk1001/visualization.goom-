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
using UTILS::MATH::U_HALF;

Circle::Circle(const FxHelper& fxHelper,
               const Helper& helper,
               const Params& circleParams,
               const PathParams& pathParams)
  : m_draw{fxHelper.GetDraw()},
    m_goomInfo{fxHelper.GetGoomInfo()},
    m_goomRand{fxHelper.GetGoomRand()},
    m_helper{helper},
    m_circleCentreTarget{circleParams.circleCentreTarget},
    m_dotPaths{NUM_DOTS,
               {static_cast<int32_t>(U_HALF * m_draw.GetScreenWidth()),
                static_cast<int32_t>(U_HALF * m_draw.GetScreenHeight())},
               circleParams.circleRadius,
               circleParams.circleCentreTarget,
               pathParams},
    m_dotDiameters{m_goomRand, NUM_DOTS, m_helper.minDotDiameter, m_helper.maxDotDiameter},
    m_lastDrawnDots(NUM_DOTS),
    m_colorMaps{GetAllSlimMaps(m_goomRand)},
    m_lowColorMaps{m_colorMaps},
    m_colorMapsGrid{NUM_DOTS, m_colorMaps->GetRandomColorMap(), m_dotPaths.GetPositionTRef(),
                    GetVerticalColorMaps(), [this](const size_t i) { return GetColorMixT(i); }},
    m_lowColorMapsGrid{NUM_DOTS, m_lowColorMaps->GetRandomColorMap(), m_dotPaths.GetPositionTRef(),
                       GetVerticalLowColorMaps(),
                       [this](const size_t i) { return GetColorMixT(i); }}
{
}

inline auto Circle::GetColorMixT([[maybe_unused]] const size_t colorIndex) const -> float
{
  return m_currentColorGridMixT;
}

auto Circle::GetVerticalColorMaps() const -> std::vector<const COLOR::IColorMap*>
{
  std::vector<const COLOR::IColorMap*> colorMaps(NUM_DOTS);

  colorMaps.at(0) = &m_colorMaps->GetRandomColorMap();
  for (size_t i = 1; i < NUM_DOTS; ++i)
  {
    colorMaps.at(i) = colorMaps.at(0);
  }

  return colorMaps;
}

auto Circle::GetVerticalLowColorMaps() const -> std::vector<const COLOR::IColorMap*>
{
  std::vector<const COLOR::IColorMap*> colorMaps(NUM_DOTS);

  colorMaps.at(0) = &m_lowColorMaps->GetRandomColorMap();
  for (size_t i = 1; i < NUM_DOTS; ++i)
  {
    colorMaps.at(i) = colorMaps.at(0);
  }

  return colorMaps;
}

void Circle::SetWeightedColorMaps(const std::shared_ptr<RandomColorMaps> weightedMaps,
                                  const std::shared_ptr<RandomColorMaps> weightedLowMaps)
{
  m_colorMaps = weightedMaps;
  m_lowColorMaps = weightedLowMaps;

  const IColorMap& newColorMap = m_colorMaps->GetRandomColorMap();
  m_colorMapsGrid.SetColorMaps(newColorMap, GetVerticalColorMaps());
  const IColorMap& newLowColorMap = m_lowColorMaps->GetRandomColorMap();
  m_lowColorMapsGrid.SetColorMaps(newLowColorMap, GetVerticalLowColorMaps());
  m_currentColorGridMixT = m_goomRand.GetRandInRange(0.0F, 1.0F);

  m_linesColorMap = &newColorMap;
  m_linesLowColorMap = &newLowColorMap;
  constexpr float PROB_SHOW_LINE = 0.05F;
  m_showLine = m_goomRand.ProbabilityOf(PROB_SHOW_LINE);

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
  // nothing to do
}

void Circle::UpdateAndDraw()
{
  UpdateTime();
  UpdatePositionSpeed();
  DrawNextCircle();
}

inline void Circle::UpdateTime()
{
  ++m_updateNum;
  m_blankTimer.Increment();
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
}

void Circle::DrawNextCircleDots()
{
  const std::vector<Point2dInt> nextDotPositions = m_dotPaths.GetNextDotPositions();

  const float brightness = GetCurrentBrightness();
  const float dotBrightness = GetDotBrightness(brightness);
  const float lineBrightness = GetLineBrightness(brightness);
  const std::vector<Pixel> dotColors = GetDotColors(dotBrightness);
  const std::vector<Pixel> dotLowColors = GetDotLowColors(dotBrightness);

  float tLineColor = 0.0F;
  Point2dInt prevDotPosition = nextDotPositions[NUM_DOTS - 1];
  for (size_t i = 0; i < NUM_DOTS; ++i)
  {
    const Point2dInt dotPosition = nextDotPositions.at(i);
    const uint32_t dotDiameter = m_dotDiameters.GetDotDiameters().at(i);
    const Pixel dotColor = dotColors.at(i);
    const Pixel dotLowColor = dotLowColors.at(i);

    DrawDot(dotPosition, dotDiameter, dotColor, dotLowColor);
    DrawLine(prevDotPosition, dotPosition, lineBrightness, tLineColor);

    m_lastDrawnDots.at(i) = dotPosition;
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

inline auto Circle::GetDotColors(const float dotBrightness) const -> std::vector<Pixel>
{
  std::vector<Pixel> dotColors = m_colorMapsGrid.GetNextColors();

  for (auto& color : dotColors)
  {
    color = GetFinalColor(dotBrightness, color);
  }

  return dotColors;
}

inline auto Circle::GetDotLowColors(const float dotBrightness) const -> std::vector<Pixel>
{
  constexpr float BRIGHTNESS_INCREASE = 1.1F;

  std::vector<Pixel> dotColors = m_lowColorMapsGrid.GetNextColors();

  for (auto& color : dotColors)
  {
    color = GetFinalColor(BRIGHTNESS_INCREASE * dotBrightness, color);
  }

  return dotColors;
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
    const Pixel color = GetFinalColor(lineBrightness, m_linesColorMap->GetColor(tDotColor));
    const Pixel lowColor =
        GetFinalLowColor(lineBrightness, m_linesLowColorMap->GetColor(tDotColor));

    DrawDot(dotPos, m_helper.minDotDiameter, color, lowColor);

    tDotPos += T_DOT_POS_STEP;
    tDotColor += T_DOT_COLOR_STEP;
  }

  if (m_showLine)
  {
    constexpr uint8_t LINE_THICKNESS = 1;
    const Pixel color = GetFinalColor(lineBrightness, m_linesColorMap->GetColor(tDotColor));
    const Pixel lowColor =
        GetFinalLowColor(lineBrightness, m_linesLowColorMap->GetColor(tDotColor));
    m_draw.Line(pos1.x, pos1.y, pos2.x, pos2.y, {color, lowColor}, LINE_THICKNESS);
  }
}

} // namespace GOOM::VISUAL_FX::CIRCLES
