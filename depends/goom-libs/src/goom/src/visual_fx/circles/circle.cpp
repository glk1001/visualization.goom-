#include "circle.h"

#undef NO_LOGGING

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
using COLOR::GetIncreasedChroma;
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
using UTILS::MATH::OscillatingPath;
using UTILS::MATH::U_HALF;

class Circle::DotDrawer
{
public:
  DotDrawer(IGoomDraw& draw, const IGoomRand& goomRand, const Helper& helper);

  void SetWeightedColorMaps(const RandomColorMaps& weightedMaps);

  void DrawDot(const Point2dInt& pos, uint32_t diameter, const Pixel& color, const Pixel& lowColor);

private:
  DRAW::IGoomDraw& m_draw;
  const IGoomRand& m_goomRand;
  const Helper& m_helper;
  static constexpr float MIN_BGND_MIX_T = 0.2F;
  static constexpr float MAX_BGND_MIX_T = 0.8F;
  float m_bgndColorMixT;
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
  Pixel m_differentColor = Pixel::BLACK;

  [[nodiscard]] auto GetRandomDifferentColor(const RandomColorMaps& weightedMaps) const -> Pixel;
  [[nodiscard]] auto GetRandomDecorationType() const -> DecorationType;
  [[nodiscard]] static auto IsSpecialPoint(size_t x, size_t y, uint32_t diameter) -> bool;
  [[nodiscard]] auto GetDotMixedColor(size_t x,
                                      size_t y,
                                      uint32_t diameter,
                                      const Pixel& bgnd,
                                      const Pixel& color,
                                      float mixT) const -> Pixel;
};

Circle::Circle(const FxHelper& fxHelper,
               const Helper& helper,
               const Params& circleParams,
               const OscillatingPath::Params& pathParams) noexcept
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
    m_colorMaps{GetAllSlimMaps(m_goomRand)},
    m_lowColorMaps{m_colorMaps},
    m_colorMapsGrid{NUM_DOTS, m_colorMaps->GetRandomColorMap(), m_dotPaths.GetPositionTRef(),
                    GetVerticalColorMaps(), [this](const size_t i) { return GetColorMixT(i); }},
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
  TValue positionT{TValue::StepType::SINGLE_CYCLE, NUM_DOTS};
  const AngleParams circleAngleParams{0.0F, UTILS::MATH::DEGREES_360};
  const auto path = std::make_unique<CirclePath>(centre, positionT, radius, circleAngleParams);

  std::vector<Point2dInt> dotStartingPositions(NUM_DOTS);

  for (size_t i = 0; i < NUM_DOTS; ++i)
  {
    dotStartingPositions.at(i) = path->GetNextPoint();

    positionT.Increment();
  }

  return dotStartingPositions;
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
  static constexpr float PROB_SHOW_LINE = 0.05F;
  m_showLine = m_goomRand.ProbabilityOf(PROB_SHOW_LINE);

  m_dotDiameters.ChangeDotDiameters();
  m_dotDrawer->SetWeightedColorMaps(*weightedMaps);
}

void Circle::SetZoomMidpoint([[maybe_unused]] const Point2dInt& zoomMidpoint)
{
  // Don't need the zoom midpoint.
}

void Circle::SetPathParams(const OscillatingPath::Params& pathParams)
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

  if ((!m_dotPaths.GetPositionTRef().IsDelayed()) && m_dotPaths.HasPositionTJustHitStartBoundary())
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
  const std::vector<Pixel> dotColors = GetDotColors(dotBrightness);
  const std::vector<Pixel> dotLowColors = GetDotLowColors(dotBrightness);

  uint32_t iRotate = m_dotAttributeOffset;
  float tLineColor = 0.0F;
  Point2dInt prevDotPosition = nextDotPositions[NUM_DOTS - 1];
  for (size_t i = 0; i < NUM_DOTS; ++i)
  {
    const Point2dInt dotPosition = nextDotPositions.at(i);
    const uint32_t dotDiameter = m_dotDiameters.GetDotDiameters().at(iRotate);
    const Pixel dotColor = dotColors.at(iRotate);
    const Pixel dotLowColor = dotLowColors.at(iRotate);

    m_dotDrawer->DrawDot(dotPosition, dotDiameter, dotColor, dotLowColor);
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
  static constexpr float INNER_POSITION_MARGIN = 0.05F;
  return m_dotPaths.GetPositionT() < (1.0F - INNER_POSITION_MARGIN) ? 1.0F : BRIGHTNESS_CUT;
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
  static constexpr float BRIGHTNESS_INCREASE = 2.0F;
  return BRIGHTNESS_INCREASE * brightness;
}

inline auto Circle::IsSpecialUpdateNum() const -> bool
{
  static constexpr uint64_t SPECIAL_UPDATE_MULTIPLE = 5;
  return 0 == (m_updateNum % SPECIAL_UPDATE_MULTIPLE);
}

inline auto Circle::GetLineBrightness(const float brightness) const -> float
{
  static constexpr float BRIGHTNESS_INCREASE = 10.0F;
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
  static constexpr float BRIGHTNESS_INCREASE = 1.1F;

  std::vector<Pixel> dotColors = m_lowColorMapsGrid.GetNextColors();

  for (auto& color : dotColors)
  {
    color = GetFinalColor(BRIGHTNESS_INCREASE * dotBrightness, color);
  }

  return dotColors;
}

inline auto Circle::GetFinalColor(const float brightness, const Pixel& color) const -> Pixel
{
  static constexpr float COLOR_BRIGHTNESS = 15.0F;
  return GetCorrectedColor(brightness * COLOR_BRIGHTNESS, color);
}

inline auto Circle::GetFinalLowColor(const float brightness, const Pixel& lowColor) const -> Pixel
{
  static constexpr float LOW_COLOR_BRIGHTNESS = 15.0F;
  return GetCorrectedColor(brightness * LOW_COLOR_BRIGHTNESS, lowColor);
}

inline auto Circle::GetCorrectedColor(const float brightness, const Pixel& color) const -> Pixel
{
  return m_helper.gammaCorrect.GetCorrection(brightness, GetIncreasedChroma(color));
}

inline void Circle::DrawLine(const Point2dInt& pos1,
                             const Point2dInt& pos2,
                             const float lineBrightness,
                             const float tLineColor)
{
  static constexpr uint32_t NUM_LINE_DOTS = 5;

  static constexpr float T_DOT_POS_STEP = 1.0F / static_cast<float>(NUM_LINE_DOTS);
  float tDotPos = T_DOT_POS_STEP;

  static constexpr float T_DOT_COLOR_STEP = T_LINE_COLOR_STEP / static_cast<float>(NUM_LINE_DOTS);

  float tDotColor = tLineColor + T_DOT_COLOR_STEP;
  for (uint32_t i = 0; i < (NUM_LINE_DOTS - 1); ++i)
  {
    const Point2dInt dotPos = lerp(pos1, pos2, tDotPos);
    const Pixel color = GetFinalColor(lineBrightness, m_linesColorMap->GetColor(tDotColor));
    const Pixel lowColor =
        GetFinalLowColor(lineBrightness, m_linesLowColorMap->GetColor(tDotColor));

    m_dotDrawer->DrawDot(dotPos, m_helper.lineDotDiameter, color, lowColor);

    tDotPos += T_DOT_POS_STEP;
    tDotColor += T_DOT_COLOR_STEP;
  }

  if (m_showLine)
  {
    static constexpr uint8_t LINE_THICKNESS = 1;
    const Pixel color = GetFinalColor(lineBrightness, m_linesColorMap->GetColor(tDotColor));
    const Pixel lowColor =
        GetFinalLowColor(lineBrightness, m_linesLowColorMap->GetColor(tDotColor));
    m_draw.Line(pos1, pos2, {color, lowColor}, LINE_THICKNESS);
  }
}

Circle::DotDrawer::DotDrawer(DRAW::IGoomDraw& draw,
                             const IGoomRand& goomRand,
                             const Circle::Helper& helper)
  : m_draw{draw},
    m_goomRand{goomRand},
    m_helper{helper},
    m_bgndColorMixT{m_goomRand.GetRandInRange(MIN_BGND_MIX_T, MAX_BGND_MIX_T)},
    m_bgndLowColorMixT{m_goomRand.GetRandInRange(MIN_BGND_MIX_T, MAX_BGND_MIX_T)},
    m_decorationType{GetRandomDecorationType()},
    m_differentColor{GetRandomDifferentColor(RandomColorMaps{m_goomRand})}
{
}

inline void Circle::DotDrawer::DrawDot(const Point2dInt& pos,
                                       const uint32_t diameter,
                                       const Pixel& color,
                                       const Pixel& lowColor)
{
  const auto getColor = [this, &color, &diameter](const size_t x, const size_t y, const Pixel& bgnd)
  { return GetDotMixedColor(x, y, diameter, bgnd, color, m_bgndColorMixT); };

  const auto getLowColor =
      [this, &lowColor, &diameter](const size_t x, const size_t y, const Pixel& bgnd)
  { return GetDotMixedColor(x, y, diameter, bgnd, lowColor, m_bgndLowColorMixT); };

  m_draw.Bitmap(pos, m_helper.bitmapGetter.GetBitmap(diameter), {getColor, getLowColor});
}

inline void Circle::DotDrawer::SetWeightedColorMaps(const RandomColorMaps& weightedMaps)
{
  m_bgndColorMixT = m_goomRand.GetRandInRange(MIN_BGND_MIX_T, MAX_BGND_MIX_T);
  m_bgndLowColorMixT = m_goomRand.GetRandInRange(MIN_BGND_MIX_T, MAX_BGND_MIX_T);
  m_decorationType = GetRandomDecorationType();
  m_differentColor = GetRandomDifferentColor(weightedMaps);
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
    return Pixel::BLACK;
  }

  const Pixel mixedColor = IColorMap::GetColorMix(bgnd, color, mixT);

  if (!IsSpecialPoint(x, y, diameter))
  {
    return mixedColor;
  }

  static constexpr float DIFFERENT_COLOR_BRIGHTNESS = 1.5F;
  static constexpr float SPECIAL_BRIGHTNESS = 1.5F;

  switch (m_decorationType)
  {
    case DecorationType::NO_DECORATION:
      return mixedColor;
    case DecorationType::BLACK_LINES:
      return Pixel::BLACK;
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
  if ((0 == x) || ((diameter - 1) == x) || (0 == y) || ((diameter - 1) == y))
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
