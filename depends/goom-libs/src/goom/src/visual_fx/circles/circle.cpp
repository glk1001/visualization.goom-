#include "circle.h"

//#undef NO_LOGGING

#include "color/colormaps.h"
#include "color/colorutils.h"
#include "color/random_colormaps.h"
#include "draw/goom_draw.h"
#include "goom_graphic.h"
#include "logging.h"
#include "point2d.h"
#include "utils/enumutils.h"
#include "utils/math/goom_rand_base.h"
#include "utils/math/misc.h"
#include "utils/math/paths.h"
#include "utils/t_values.h"
#include "utils/timer.h"

#include <cmath>
#include <cstdint>
#include <memory>
#include <vector>

namespace GOOM::VISUAL_FX::CIRCLES
{

using COLOR::GetAllSlimMaps;
using COLOR::GetBrighterColor;
using COLOR::IColorMap;
using COLOR::RandomColorMaps;
using DRAW::IGoomDraw;
using UTILS::Logging;
using UTILS::NUM;
using UTILS::Timer;
using UTILS::TValue;
using UTILS::MATH::AngleParams;
using UTILS::MATH::CirclePath;
using UTILS::MATH::IGoomRand;
using UTILS::MATH::IsEven;
using UTILS::MATH::ModIncrement;
using UTILS::MATH::OscillatingFunction;
using UTILS::MATH::OscillatingPath;
using UTILS::MATH::U_HALF;

class Circle::DotDrawer
{
public:
  DotDrawer(IGoomDraw& draw, const IGoomRand& goomRand, const Helper& helper);

  void SetWeightedColorMaps(const RandomColorMaps& weightedMaps);

  auto DrawDot(const Point2dInt& pos,
               uint32_t diameter,
               const Pixel& mainColor,
               const Pixel& lowColor) -> void;

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
    _num // unused and must be last
  };
  DecorationType m_decorationType;
  Pixel m_differentColor = BLACK_PIXEL;

  bool m_doCircleDotShapes = true;
  const IColorMap* m_outerCircleDotColorMap{};
  static constexpr float MIN_OUTER_CIRCLE_DOT_COLOR_MIX_T = 0.1F;
  static constexpr float MAX_OUTER_CIRCLE_DOT_COLOR_MIX_T = 0.9F;
  float m_outerCircleDotColorMix = MIN_OUTER_CIRCLE_DOT_COLOR_MIX_T;
  static constexpr float CIRCLE_DOT_LOW_COLOR_BRIGHTNESS_FACTOR = 0.1F;

  auto DrawBitmapDot(const Point2dInt& position,
                     uint32_t diameter,
                     const Pixel& mainColor,
                     const Pixel& lowColor) -> void;
  [[nodiscard]] auto GetRandomDifferentColor(const RandomColorMaps& weightedMaps) const -> Pixel;
  [[nodiscard]] auto GetRandomDecorationType() const -> DecorationType;
  [[nodiscard]] static auto IsSpecialPoint(size_t x, size_t y, uint32_t diameter) -> bool;
  [[nodiscard]] auto GetDotMixedColor(size_t x,
                                      size_t y,
                                      uint32_t diameter,
                                      const Pixel& bgnd,
                                      const Pixel& color,
                                      float mixT) const -> Pixel;
  auto DrawCircleDot(const Point2dInt& centre,
                     uint32_t diameter,
                     const Pixel& mainColor,
                     const Pixel& lowColor,
                     const IColorMap& innerColorMap) noexcept -> void;
  [[nodiscard]] static auto GetCircleColors(float brightness,
                                            const Pixel& mainColor,
                                            const Pixel& lowColor) noexcept -> std::vector<Pixel>;
  [[nodiscard]] static auto GetCircleColorsWithInner(float brightness,
                                                     const Pixel& mainColor,
                                                     const Pixel& lowColor,
                                                     const Pixel& innerColor,
                                                     float innerColorMix) noexcept
      -> std::vector<Pixel>;
};

Circle::Circle(const FxHelper& fxHelper,
               const Helper& helper,
               const Params& circleParams,
               const OscillatingFunction::Params& pathParams) noexcept
  : m_draw{fxHelper.GetDraw()},
    m_goomInfo{fxHelper.GetGoomInfo()},
    m_goomRand{fxHelper.GetGoomRand()},
    m_helper{helper},
    m_circleCentreTarget{circleParams.circleCentreTarget},
    m_dotPaths{m_goomRand, NUM_DOTS,
               GetDotStartingPositions({static_cast<int32_t>(U_HALF * m_draw.GetScreenWidth()),
                                        static_cast<int32_t>(U_HALF * m_draw.GetScreenHeight())},
                                       circleParams.circleRadius),
               circleParams.circleCentreTarget, pathParams},
    m_dotDiameters{m_goomRand, NUM_DOTS, m_helper.minDotDiameter, m_helper.maxDotDiameter},
    m_lastDrawnDots(NUM_DOTS),
    m_dotDrawer{std::make_unique<DotDrawer>(m_draw, m_goomRand, m_helper)},
    m_mainColorMaps{GetAllSlimMaps(m_goomRand)},
    m_lowColorMaps{m_mainColorMaps},
    m_mainColorMapsGrid{NUM_DOTS, m_mainColorMaps->GetRandomColorMap(),
                        m_dotPaths.GetPositionTRef(), GetVerticalMainColorMaps(),
                        [this](const size_t i) { return GetColorMixT(i); }},
    m_lowColorMapsGrid{NUM_DOTS, m_lowColorMaps->GetRandomColorMap(), m_dotPaths.GetPositionTRef(),
                       GetVerticalLowColorMaps(),
                       [this](const size_t i) { return GetColorMixT(i); }}
{
}

Circle::Circle(Circle&&) noexcept = default;

Circle::~Circle() noexcept = default;

auto Circle::GetDotStartingPositions(const Point2dInt& centre, const float radius)
    -> std::vector<Point2dInt>
{
  auto positionT = std::make_unique<TValue>(TValue::StepType::SINGLE_CYCLE, NUM_DOTS);
  const Vec2dFlt centrePos{centre.ToFlt()};
  static constexpr AngleParams DEFAULT_ANGLE_PARAMS{};
  const auto path =
      std::make_unique<CirclePath>(std::move(positionT), centrePos, radius, DEFAULT_ANGLE_PARAMS);

  std::vector<Point2dInt> dotStartingPositions(NUM_DOTS);

  for (size_t i = 0; i < NUM_DOTS; ++i)
  {
    dotStartingPositions.at(i) = path->GetNextPoint();

    path->IncrementT();
  }

  return dotStartingPositions;
}

inline auto Circle::GetColorMixT([[maybe_unused]] const size_t colorIndex) const -> float
{
  return m_currentColorGridMixT;
}

auto Circle::GetVerticalMainColorMaps() const -> std::vector<const COLOR::IColorMap*>
{
  std::vector<const COLOR::IColorMap*> colorMaps(NUM_DOTS);

  colorMaps.at(0) = &m_mainColorMaps->GetRandomColorMap();
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

void Circle::SetWeightedColorMaps(const std::shared_ptr<RandomColorMaps> weightedMainMaps,
                                  const std::shared_ptr<RandomColorMaps> weightedLowMaps)
{
  m_mainColorMaps = weightedMainMaps;
  m_lowColorMaps = weightedLowMaps;

  const IColorMap& newMainColorMap = m_mainColorMaps->GetRandomColorMap();
  m_mainColorMapsGrid.SetColorMaps(newMainColorMap, GetVerticalMainColorMaps());
  const IColorMap& newLowColorMap = m_lowColorMaps->GetRandomColorMap();
  m_lowColorMapsGrid.SetColorMaps(newLowColorMap, GetVerticalLowColorMaps());
  m_currentColorGridMixT = m_goomRand.GetRandInRange(0.0F, 1.0F);

  m_linesMainColorMap = &newMainColorMap;
  m_linesLowColorMap = &newLowColorMap;
  static constexpr float PROB_SHOW_LINE = 0.5F;
  m_showLine = m_goomRand.ProbabilityOf(PROB_SHOW_LINE);

  m_dotDiameters.ChangeDotDiameters();
  m_dotDrawer->SetWeightedColorMaps(*weightedMainMaps);
}

void Circle::SetZoomMidpoint([[maybe_unused]] const Point2dInt& zoomMidpoint)
{
  // Don't need the zoom midpoint.
}

void Circle::SetPathParams(const OscillatingFunction::Params& pathParams)
{
  m_dotPaths.SetPathParams(pathParams);
}

void Circle::Start()
{
  m_updateNum = 0;
  m_blankTimer.SetToFinished();
  m_dotPaths.Reset();
}

void Circle::UpdateAndDraw()
{
  UpdateTime();
  DrawNextCircle();
}

inline void Circle::UpdateTime()
{
  ++m_updateNum;
  m_blankTimer.Increment();
}

inline void Circle::DrawNextCircle()
{
  if (!m_blankTimer.Finished())
  {
    return;
  }

  DrawNextCircleDots();
  ResetNumSteps();
  ResetCircleParams();
  IncrementTs();
}

inline void Circle::IncrementTs()
{
  m_dotPaths.IncrementPositionT();
  m_dotAttributeOffset = ModIncrement(m_dotAttributeOffset, NUM_DOTS);

  if ((!m_dotPaths.IsDelayed()) && m_dotPaths.HasPositionTJustHitStartBoundary())
  {
    m_blankTimer.ResetToZero();
  }
}

void Circle::ResetNumSteps()
{
  if (0 == m_newNumSteps)
  {
    return;
  }
  if ((!m_dotPaths.HasPositionTJustHitStartBoundary()) &&
      (!m_dotPaths.HasPositionTJustHitEndBoundary()))
  {
    return;
  }
  m_dotPaths.SetPositionTNumSteps(m_newNumSteps);
  m_newNumSteps = 0;
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

inline auto Circle::GetRandomCircleCentreTargetPosition() const -> Point2dInt
{
  static constexpr int32_t MARGIN = 50;
  const Point2dInt randomPosition{
      m_goomRand.GetRandInRange(MARGIN, static_cast<int32_t>(m_draw.GetScreenWidth() - MARGIN)),
      m_goomRand.GetRandInRange(MARGIN, static_cast<int32_t>(m_draw.GetScreenHeight()) - MARGIN)};

  static constexpr float TARGET_T = 0.1F;
  return lerp(m_circleCentreTarget, randomPosition, TARGET_T);
}

void Circle::DrawNextCircleDots()
{
  const std::vector<Point2dInt> nextDotPositions = m_dotPaths.GetNextDotPositions();

  const float brightness = GetCurrentBrightness();
  const float dotBrightness = GetDotBrightness(brightness);
  const float lineBrightness = GetLineBrightness(brightness);
  const std::vector<Pixel> dotMainColors = GetDotMainColors(dotBrightness);
  const std::vector<Pixel> dotLowColors = GetDotLowColors(dotBrightness);

  uint32_t iRotate = m_dotAttributeOffset;
  float tLineColor = 0.0F;
  Point2dInt prevDotPosition = nextDotPositions[NUM_DOTS - 1];
  for (size_t i = 0; i < NUM_DOTS; ++i)
  {
    const Point2dInt dotPosition = nextDotPositions.at(i);
    const uint32_t dotDiameter = m_dotDiameters.GetDotDiameters().at(iRotate);
    const Pixel dotMainColor = dotMainColors.at(iRotate);
    const Pixel dotLowColor = dotLowColors.at(iRotate);

    m_dotDrawer->DrawDot(dotPosition, dotDiameter, dotMainColor, dotLowColor);
    DrawLine(prevDotPosition, dotPosition, lineBrightness, tLineColor);

    m_lastDrawnDots.at(i) = dotPosition;
    prevDotPosition = dotPosition;
    tLineColor += T_LINE_COLOR_STEP;
    iRotate = ModIncrement(iRotate, NUM_DOTS);
  }
}

inline auto Circle::GetCurrentBrightness() const -> float
{
  static constexpr float BRIGHTNESS_CUT = 0.001F;
  static constexpr float T_NEAR_END = 0.8F;
  return m_dotPaths.GetPositionT() < T_NEAR_END ? 1.0F : BRIGHTNESS_CUT;
}

inline auto Circle::GetDotBrightness(const float brightness) const -> float
{
  if (static constexpr float T_NEAR_START = 0.1F; m_dotPaths.GetPositionT() > T_NEAR_START)
  {
    return brightness;
  }
  if (IsEven(m_updateNum))
  {
    static constexpr float BRIGHTNESS_INCREASE = 5.0F;
    return BRIGHTNESS_INCREASE * brightness;
  }
  static constexpr float BRIGHTNESS_INCREASE = 1.5F;
  return BRIGHTNESS_INCREASE * brightness;
}

inline auto Circle::IsSpecialUpdateNum() const -> bool
{
  static constexpr uint64_t SPECIAL_UPDATE_MULTIPLE = 5;
  return 0 == (m_updateNum % SPECIAL_UPDATE_MULTIPLE);
}

inline auto Circle::IsSpecialLineUpdateNum() const -> bool
{
  static constexpr uint64_t LINE_UPDATE_MULTIPLE = 8;
  return 0 == (m_updateNum % LINE_UPDATE_MULTIPLE);
}

inline auto Circle::GetLineBrightness(const float brightness) const -> float
{
  static constexpr float BRIGHTNESS_INCREASE = 10.0F;
  return IsSpecialUpdateNum() ? (BRIGHTNESS_INCREASE * brightness) : brightness;
}

inline auto Circle::GetDotMainColors(const float dotBrightness) const -> std::vector<Pixel>
{
  std::vector<Pixel> dotColors = m_mainColorMapsGrid.GetNextColors();

  for (auto& color : dotColors)
  {
    color = GetFinalMainColor(dotBrightness, color);
  }

  return dotColors;
}

inline auto Circle::GetDotLowColors(const float dotBrightness) const -> std::vector<Pixel>
{
  static constexpr float BRIGHTNESS_INCREASE = 1.1F;

  std::vector<Pixel> dotColors = m_lowColorMapsGrid.GetNextColors();

  for (auto& color : dotColors)
  {
    color = GetFinalMainColor(BRIGHTNESS_INCREASE * dotBrightness, color);
  }

  return dotColors;
}

inline auto Circle::GetFinalMainColor(const float brightness, const Pixel& mainColor) const -> Pixel
{
  static constexpr float MAIN_COLOR_BRIGHTNESS = 1.0F;
  return GetCorrectedColor(brightness * MAIN_COLOR_BRIGHTNESS, mainColor);
}

inline auto Circle::GetFinalLowColor(const float brightness, const Pixel& lowColor) const -> Pixel
{
  static constexpr float LOW_COLOR_BRIGHTNESS = 5.0F;
  return GetCorrectedColor(brightness * LOW_COLOR_BRIGHTNESS, lowColor);
}

inline auto Circle::GetCorrectedColor(const float brightness, const Pixel& color) const -> Pixel
{
  return m_helper.colorCorrect.GetCorrection(brightness, color);
}

inline auto Circle::DrawLine(const Point2dInt& position1,
                             const Point2dInt& position2,
                             const float lineBrightness,
                             const float tLineColor) -> void
{
  const float lastTDotColor = DrawLineDots(position1, position2, lineBrightness, tLineColor);
  DrawConnectingLine(position1, position2, lineBrightness, lastTDotColor);
}

inline auto Circle::DrawLineDots(const Point2dInt& position1,
                                 const Point2dInt& position2,
                                 const float lineBrightness,
                                 const float tLineColor) -> float
{
  static constexpr uint32_t NUM_LINE_DOTS = 5;

  static constexpr float T_DOT_POS_STEP = 1.0F / static_cast<float>(NUM_LINE_DOTS);
  float tDotPos = T_DOT_POS_STEP;

  static constexpr float T_DOT_COLOR_STEP = T_LINE_COLOR_STEP / static_cast<float>(NUM_LINE_DOTS);

  float tDotColor = tLineColor + T_DOT_COLOR_STEP;
  for (uint32_t i = 0; i < (NUM_LINE_DOTS - 1); ++i)
  {
    const Point2dInt dotPos = lerp(position1, position2, tDotPos);
    const Pixel mainColor =
        GetFinalMainColor(lineBrightness, m_linesMainColorMap->GetColor(tDotColor));
    const Pixel lowColor =
        GetFinalLowColor(lineBrightness, m_linesLowColorMap->GetColor(tDotColor));

    m_dotDrawer->DrawDot(dotPos, m_helper.lineDotDiameter, mainColor, lowColor);

    tDotPos += T_DOT_POS_STEP;
    tDotColor += T_DOT_COLOR_STEP;
  }

  return tDotColor;
}

inline auto Circle::DrawConnectingLine(const Point2dInt& position1,
                                       const Point2dInt& position2,
                                       const float lineBrightness,
                                       const float tDotColor) -> void
{
  if ((not m_showLine) or (not IsSpecialLineUpdateNum()))
  {
    return;
  }

  static constexpr uint8_t LINE_THICKNESS = 1;
  const Pixel mainColor =
      GetFinalMainColor(lineBrightness, m_linesMainColorMap->GetColor(tDotColor));
  const Pixel lowColor = GetFinalLowColor(lineBrightness, m_linesLowColorMap->GetColor(tDotColor));

  m_draw.Line(position1, position2, {mainColor, lowColor}, LINE_THICKNESS);
}

Circle::DotDrawer::DotDrawer(DRAW::IGoomDraw& draw,
                             const IGoomRand& goomRand,
                             const Circle::Helper& helper)
  : m_draw{draw},
    m_goomRand{goomRand},
    m_helper{helper},
    m_bgndMainColorMixT{m_goomRand.GetRandInRange(MIN_BGND_MIX_T, MAX_BGND_MIX_T)},
    m_bgndLowColorMixT{m_goomRand.GetRandInRange(MIN_BGND_MIX_T, MAX_BGND_MIX_T)},
    m_decorationType{GetRandomDecorationType()},
    m_differentColor{GetRandomDifferentColor(RandomColorMaps{m_goomRand})}
{
}

inline void Circle::DotDrawer::SetWeightedColorMaps(const RandomColorMaps& weightedMaps)
{
  m_bgndMainColorMixT = m_goomRand.GetRandInRange(MIN_BGND_MIX_T, MAX_BGND_MIX_T);
  m_bgndLowColorMixT = m_goomRand.GetRandInRange(MIN_BGND_MIX_T, MAX_BGND_MIX_T);
  m_decorationType = GetRandomDecorationType();
  m_differentColor = GetRandomDifferentColor(weightedMaps);
  m_outerCircleDotColorMap = &weightedMaps.GetRandomColorMap();

  static constexpr float PROB_CIRCLES = 0.5F;
  m_doCircleDotShapes = m_goomRand.ProbabilityOf(PROB_CIRCLES);
  m_outerCircleDotColorMix =
      m_goomRand.GetRandInRange(MIN_OUTER_CIRCLE_DOT_COLOR_MIX_T, MAX_OUTER_CIRCLE_DOT_COLOR_MIX_T);
}

inline void Circle::DotDrawer::DrawDot(const Point2dInt& pos,
                                       const uint32_t diameter,
                                       const Pixel& mainColor,
                                       const Pixel& lowColor)
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

inline auto Circle::DotDrawer::DrawCircleDot(const Point2dInt& centre,
                                             const uint32_t diameter,
                                             const Pixel& mainColor,
                                             const Pixel& lowColor,
                                             const IColorMap& innerColorMap) noexcept -> void
{
  const auto maxRadius = static_cast<int32_t>(diameter + 3) / 2;
  TValue innerColorT{UTILS::TValue::StepType::SINGLE_CYCLE, static_cast<uint32_t>(maxRadius - 1)};
  static constexpr int32_t INNER_COLOR_CUTOFF_RADIUS = 4;

  static constexpr float MIN_BRIGHTNESS = 1.0F;
  static constexpr float MAX_BRIGHTNESS = 3.0F;
  TValue brightnessT{TValue::StepType::SINGLE_CYCLE, static_cast<uint32_t>(maxRadius)};

  // '> 1' means leave a little hole in the middle of the circles.
  for (int32_t radius = maxRadius; radius > 1; --radius)
  {
    const float brightness = STD20::lerp(MIN_BRIGHTNESS, MAX_BRIGHTNESS, brightnessT());
    const Pixel innerColor = innerColorMap.GetColor(innerColorT());
    const std::vector<Pixel> colors =
        radius <= INNER_COLOR_CUTOFF_RADIUS
            ? GetCircleColors(brightness, mainColor, lowColor)
            : GetCircleColorsWithInner(brightness, mainColor, lowColor, innerColor,
                                       m_outerCircleDotColorMix);

    m_draw.Circle(centre, radius, colors);

    brightnessT.Increment();
    innerColorT.Increment();
  }
}

inline auto Circle::DotDrawer::GetCircleColors(const float brightness,
                                               const Pixel& mainColor,
                                               const Pixel& lowColor) noexcept -> std::vector<Pixel>
{
  const Pixel finalMainColor = GetBrighterColor(brightness, mainColor);
  const Pixel finalLowColor =
      GetBrighterColor(CIRCLE_DOT_LOW_COLOR_BRIGHTNESS_FACTOR * brightness, lowColor);
  return {finalMainColor, finalLowColor};
}

inline auto Circle::DotDrawer::GetCircleColorsWithInner(const float brightness,
                                                        const Pixel& mainColor,
                                                        const Pixel& lowColor,
                                                        const Pixel& innerColor,
                                                        const float innerColorMix) noexcept
    -> std::vector<Pixel>
{
  const Pixel finalMainColor =
      GetBrighterColor(brightness, IColorMap::GetColorMix(mainColor, innerColor, innerColorMix));
  const Pixel finalLowColor =
      GetBrighterColor(CIRCLE_DOT_LOW_COLOR_BRIGHTNESS_FACTOR * brightness,
                       IColorMap::GetColorMix(lowColor, innerColor, innerColorMix));

  return {finalMainColor, finalLowColor};
}

inline auto Circle::DotDrawer::DrawBitmapDot(const Point2dInt& position,
                                             const uint32_t diameter,
                                             const Pixel& mainColor,
                                             const Pixel& lowColor) -> void
{
  const auto getMainColor =
      [this, &mainColor, &diameter](const size_t x, const size_t y, const Pixel& bgnd)
  {
    static constexpr float MAIN_COLOR_BRIGHTNESS = 1.0F;
    const Pixel color = GetBrighterColor(MAIN_COLOR_BRIGHTNESS, mainColor);
    return GetDotMixedColor(x, y, diameter, bgnd, color, m_bgndMainColorMixT);
  };

  const auto getLowColor =
      [this, &lowColor, &diameter](const size_t x, const size_t y, const Pixel& bgnd)
  {
    static constexpr float LOW_COLOR_BRIGHTNESS = 2.0F;
    const Pixel color = GetBrighterColor(LOW_COLOR_BRIGHTNESS, lowColor);
    return GetDotMixedColor(x, y, diameter, bgnd, color, m_bgndLowColorMixT);
  };

  m_draw.Bitmap(position, m_helper.bitmapGetter.GetBitmap(diameter), {getMainColor, getLowColor});
}

inline auto Circle::DotDrawer::GetRandomDecorationType() const -> DecorationType
{
  return static_cast<DecorationType>(m_goomRand.GetRandInRange(0U, NUM<DecorationType>));
}

inline auto Circle::DotDrawer::GetRandomDifferentColor(const RandomColorMaps& weightedMaps) const
    -> Pixel
{
  return weightedMaps.GetRandomColorMap().GetColor(m_goomRand.GetRandInRange(0.0F, 1.0F));
}

inline auto Circle::DotDrawer::GetDotMixedColor(const size_t x,
                                                const size_t y,
                                                const uint32_t diameter,
                                                const Pixel& bgnd,
                                                const Pixel& color,
                                                const float mixT) const -> Pixel
{
  if (0 == bgnd.A())
  {
    return BLACK_PIXEL;
  }

  const Pixel mixedColor = IColorMap::GetColorMix(bgnd, color, mixT);

  if (!IsSpecialPoint(x, y, diameter))
  {
    return mixedColor;
  }

  static constexpr float DIFFERENT_COLOR_BRIGHTNESS = 5.0F;
  static constexpr float SPECIAL_BRIGHTNESS = 5.0F;

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
      throw std::logic_error("Unknown DecorationType.");
  }
}

inline auto Circle::DotDrawer::IsSpecialPoint(const size_t x,
                                              const size_t y,
                                              const uint32_t diameter) -> bool
{
  if (static constexpr size_t EDGE_CUTOFF = 3;
      (x <= EDGE_CUTOFF) || (x >= (diameter - EDGE_CUTOFF)) || (y <= EDGE_CUTOFF) ||
      (y >= (diameter - EDGE_CUTOFF)))
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
