#include "circles_fx.h"

#include "color/colormaps.h"
#include "color/colorutils.h"
#include "color/random_colormaps.h"
#include "draw/goom_draw.h"
#include "fx_helpers.h"
#include "goom/spimpl.h"
#include "goom_graphic.h"
#include "goom_plugin_info.h"
#include "point2d.h"
#include "utils/goom_rand_base.h"
#include "utils/graphics/image_bitmaps.h"
#include "utils/graphics/small_image_bitmaps.h"
#include "utils/t_values.h"
#include "utils/timer.h"

#include <array>
#undef NDEBUG
#include <cassert>
#include <cmath>
#include <cstdint>
#include <memory>
#include <vector>

namespace GOOM::VISUAL_FX
{

using COLOR::GammaCorrection;
using COLOR::GetAllSlimMaps;
using COLOR::GetBrighterColor;
using COLOR::GetIncreasedChroma;
using COLOR::IColorMap;
using COLOR::RandomColorMaps;
using DRAW::IGoomDraw;
using UTILS::IGoomRand;
using UTILS::Timer;
using UTILS::TValue;
using UTILS::Weights;
using UTILS::GRAPHICS::ImageBitmap;
using UTILS::GRAPHICS::SmallImageBitmaps;

class CirclesFx::CirclesFxImpl
{
public:
  CirclesFxImpl(const FxHelpers& fxHelpers, const SmallImageBitmaps& smallBitmaps);

  void SetWeightedColorMaps(std::shared_ptr<COLOR::RandomColorMaps> weightedMaps);

  void SetZoomMidPoint(const Point2dInt& zoomMidPoint);

  void Start();
  void ApplyMultiple();

private:
  IGoomDraw& m_draw;
  const PluginInfo& m_goomInfo;
  const IGoomRand& m_goomRand;
  const SmallImageBitmaps& m_smallBitmaps;

  std::shared_ptr<RandomColorMaps> m_colorMaps;
  [[nodiscard]] auto GetRandomColorMap() const -> const IColorMap&;

  uint64_t m_updateNum = 0;
  static constexpr uint32_t BLANK_TIME = 40;
  Timer m_blankTimer{BLANK_TIME, true};
  void UpdateTime();

  static constexpr uint32_t NUM_CIRCLE_PATHS = 30;
  static_assert(0 == (NUM_CIRCLE_PATHS % 2));
  static constexpr uint32_t DEFAULT_POSITION_STEPS = 100;
  static constexpr uint32_t MIN_POSITION_STEPS = 50;
  static constexpr uint32_t MAX_POSITION_STEPS = 250;
  static constexpr uint32_t DELAY_TIME_AT_CENTRE = 20;
  static constexpr uint32_t DELAY_TIME_AT_EDGE = 10;
  TValue m_positionT{
      TValue::StepType::CONTINUOUS_REVERSIBLE,
      DEFAULT_POSITION_STEPS,
      {{0.0F, DELAY_TIME_AT_EDGE}, {1.0F, DELAY_TIME_AT_CENTRE}},
      0.0F
  };
  Point2dInt m_centreTargetPosition;
  Point2dInt m_zoomMidPoint;
  static constexpr size_t MIN_DIAMETER = 5;
  static constexpr size_t MAX_DIAMETER = 21;
  std::array<uint32_t, NUM_CIRCLE_PATHS> m_circleDiameters;
  [[nodiscard]] static auto GetInitialCircleDiameters() -> std::array<uint32_t, NUM_CIRCLE_PATHS>;
  std::array<Point2dInt, NUM_CIRCLE_PATHS> m_circleRandomStartingPositions;
  const std::array<Point2dInt, NUM_CIRCLE_PATHS> m_circleStartingPositions;
  [[nodiscard]] static auto GetRandomStartingCirclePositions(const IGoomRand& goomRand,
                                                             int32_t width,
                                                             int32_t height)
      -> std::array<Point2dInt, NUM_CIRCLE_PATHS>;
  [[nodiscard]] static auto GetStartingCirclePositions(const IGoomRand& goomRand,
                                                       int32_t width,
                                                       int32_t height)
      -> std::array<Point2dInt, NUM_CIRCLE_PATHS>;
  static constexpr uint32_t NUM_COLOR_STEPS = 20;
  std::vector<TValue> m_colorTs;
  [[nodiscard]] static auto GetInitialColorTs() -> std::vector<TValue>;
  void UpdateColorTs();
  std::array<const IColorMap*, NUM_CIRCLE_PATHS> m_circleColorMaps{};
  std::array<const IColorMap*, NUM_CIRCLE_PATHS> m_circleLowColorMaps{};
  const IColorMap* m_linesColorMap{};
  void ChangeCircleColorMaps();
  void ChangeCircleDiameters();
  void UpdateSpeeds();
  void ResetCentreTargetPosition();
  static constexpr float POSITION_MARGIN = 0.05F;
  void DrawNextCircles();
  void DrawTheCircles();
  void UpdateTs();
  [[nodiscard]] auto GetCircleCentrePositions() const -> std::array<Point2dInt, NUM_CIRCLE_PATHS>;
  [[nodiscard]] auto GetNextTargetPosition() const -> Point2dInt;
  void DrawCircle(const Point2dInt& centre,
                  uint32_t diameter,
                  const Pixel& color,
                  const Pixel& lowColor);
  void DrawLine(const Point2dInt& pos1,
                const Point2dInt& pos2,
                const Pixel& color,
                const Pixel& lowColor,
                uint8_t lineThickness);
  [[nodiscard]] auto GetFinalColor(float brightness, const Pixel& color) const -> Pixel;
  [[nodiscard]] auto GetFinalLowColor(float brightness, const Pixel& lowColor) const -> Pixel;
  [[nodiscard]] auto GetCorrectedColor(float brightness, const Pixel& color) const -> Pixel;
  SmallImageBitmaps::ImageNames m_currentBitmapName{SmallImageBitmaps::ImageNames::SPHERE};
  const Weights<SmallImageBitmaps::ImageNames> m_bitmapTypes;
  [[nodiscard]] auto GetImageBitmap(size_t size) const -> const ImageBitmap&;

  static constexpr float GAMMA = 1.0F / 2.2F;
  static constexpr float GAMMA_BRIGHTNESS_THRESHOLD = 0.01F;
  GammaCorrection m_gammaCorrect{GAMMA, GAMMA_BRIGHTNESS_THRESHOLD};
  auto GetGammaCorrection(float brightness, const Pixel& color) const -> Pixel;
};

CirclesFx::CirclesFx(const FxHelpers& fxHelpers, const SmallImageBitmaps& smallBitmaps) noexcept
  : m_fxImpl{spimpl::make_unique_impl<CirclesFxImpl>(fxHelpers, smallBitmaps)}
{
}

void CirclesFx::SetWeightedColorMaps(std::shared_ptr<COLOR::RandomColorMaps> weightedMaps)
{
  m_fxImpl->SetWeightedColorMaps(weightedMaps);
}

void CirclesFx::SetZoomMidPoint(const Point2dInt& zoomMidPoint)
{
  m_fxImpl->SetZoomMidPoint(zoomMidPoint);
}

void CirclesFx::Start()
{
  m_fxImpl->Start();
}

void CirclesFx::Finish()
{
  // nothing to do
}

auto CirclesFx::GetFxName() const -> std::string
{
  return "circles";
}

void CirclesFx::ApplyMultiple()
{
  m_fxImpl->ApplyMultiple();
}

// clang-format off
constexpr float IMAGE_NAMES_CIRCLE_WEIGHT        =  5.0F;
constexpr float IMAGE_NAMES_SPHERE_WEIGHT        = 10.0F;
constexpr float IMAGE_NAMES_ORANGE_FLOWER_WEIGHT = 10.0F;
constexpr float IMAGE_NAMES_PINK_FLOWER_WEIGHT   =  5.0F;
constexpr float IMAGE_NAMES_RED_FLOWER_WEIGHT    = 10.0F;
constexpr float IMAGE_NAMES_WHITE_FLOWER_WEIGHT  =  5.0F;
// clang-format on

CirclesFx::CirclesFxImpl::CirclesFxImpl(const FxHelpers& fxHelpers,
                                        const SmallImageBitmaps& smallBitmaps)
  : m_draw{fxHelpers.GetDraw()},
    m_goomInfo{fxHelpers.GetGoomInfo()},
    m_goomRand{fxHelpers.GetGoomRand()},
    m_smallBitmaps{smallBitmaps},
    m_colorMaps{GetAllSlimMaps(m_goomRand)},
    m_centreTargetPosition{m_draw.GetScreenWidth() / 2, m_draw.GetScreenHeight() / 2},
    m_zoomMidPoint{m_centreTargetPosition},
    m_circleDiameters{GetInitialCircleDiameters()},
    m_circleRandomStartingPositions{
        GetRandomStartingCirclePositions(m_goomRand,
                                         static_cast<int32_t>(m_draw.GetScreenWidth()),
                                         static_cast<int32_t>(m_draw.GetScreenHeight()))},
    m_circleStartingPositions{
        GetStartingCirclePositions(m_goomRand,
                                   static_cast<int32_t>(m_draw.GetScreenWidth()),
                                   static_cast<int32_t>(m_draw.GetScreenHeight()))},
    m_colorTs{GetInitialColorTs()},
    // clang-format off
    m_bitmapTypes{
      m_goomRand,
      {
          {SmallImageBitmaps::ImageNames::CIRCLE,        IMAGE_NAMES_CIRCLE_WEIGHT},
          {SmallImageBitmaps::ImageNames::SPHERE,        IMAGE_NAMES_SPHERE_WEIGHT},
          {SmallImageBitmaps::ImageNames::ORANGE_FLOWER, IMAGE_NAMES_ORANGE_FLOWER_WEIGHT},
          {SmallImageBitmaps::ImageNames::PINK_FLOWER,   IMAGE_NAMES_PINK_FLOWER_WEIGHT},
          {SmallImageBitmaps::ImageNames::RED_FLOWER,    IMAGE_NAMES_RED_FLOWER_WEIGHT},
          {SmallImageBitmaps::ImageNames::WHITE_FLOWER,  IMAGE_NAMES_WHITE_FLOWER_WEIGHT},
      }
    }
// clang-format on
{
}

auto CirclesFx::CirclesFxImpl::GetInitialCircleDiameters() -> std::array<uint32_t, NUM_CIRCLE_PATHS>
{
  std::array<uint32_t, NUM_CIRCLE_PATHS> diameters{};
  diameters.fill(MAX_DIAMETER);
  return diameters;
}

auto CirclesFx::CirclesFxImpl::GetInitialColorTs() -> std::vector<TValue>
{
  std::vector<TValue> colorTs{};
  for (size_t i = 0; i < NUM_CIRCLE_PATHS; ++i)
  {
    colorTs.emplace_back(TValue::StepType::CONTINUOUS_REVERSIBLE, NUM_COLOR_STEPS, 0.0F);
  }
  return colorTs;
}

auto CirclesFx::CirclesFxImpl::GetRandomStartingCirclePositions(const IGoomRand& goomRand,
                                                                const int32_t width,
                                                                const int32_t height)
    -> std::array<Point2dInt, NUM_CIRCLE_PATHS>
{
  constexpr int32_t MARGIN = 50;

  std::array<Point2dInt, NUM_CIRCLE_PATHS> randomStartingCirclePositions{
      GetStartingCirclePositions(goomRand, width, height)};
  for (size_t i = 0; i < NUM_CIRCLE_PATHS; ++i)
  {
    randomStartingCirclePositions.at(i).Translate(
        {goomRand.GetRandInRange(MARGIN, width - MARGIN),
         goomRand.GetRandInRange(MARGIN, height - MARGIN)});
  }

  return randomStartingCirclePositions;
}

auto CirclesFx::CirclesFxImpl::GetStartingCirclePositions(
    [[maybe_unused]] const IGoomRand& goomRand, const int32_t width, const int32_t height)
    -> std::array<Point2dInt, NUM_CIRCLE_PATHS>
{
  //  const float aRadius = (0.5F * static_cast<float>(width - MARGIN));
  const float aRadius = (0.5F * static_cast<float>(height));
  const float bRadius = (0.5F * static_cast<float>(height));
  const Point2dInt centre = {width / 2, height / 2};

  std::array<Point2dInt, NUM_CIRCLE_PATHS> startingCirclePositions{};
  const float angleStep = UTILS::m_two_pi / static_cast<float>(NUM_CIRCLE_PATHS);
  float angle = 0.0F;
  for (size_t i = 0; i < NUM_CIRCLE_PATHS; ++i)
  {
    const auto x = static_cast<int32_t>(std::lround(aRadius * std::cos(angle)));
    const auto y = static_cast<int32_t>(std::lround(bRadius * std::sin(angle)));

    startingCirclePositions.at(i) = centre + Vec2dInt{x, y};

    angle += angleStep;
  }

  return startingCirclePositions;
}

void CirclesFx::CirclesFxImpl::ChangeCircleColorMaps()
{
  m_circleColorMaps.at(0) = &GetRandomColorMap();
  m_circleLowColorMaps.at(0) = m_circleColorMaps.at(0);

  for (size_t i = 1; i < NUM_CIRCLE_PATHS; ++i)
  {
    m_circleColorMaps.at(i) = m_circleColorMaps.at(0);
    m_circleLowColorMaps.at(i) = m_circleLowColorMaps.at(0);
  }

  m_linesColorMap = m_circleColorMaps.at(0);

  m_currentBitmapName = m_bitmapTypes.GetRandomWeighted();
}

void CirclesFx::CirclesFxImpl::ChangeCircleDiameters()
{
  const uint32_t fixedCircleDiameter = m_goomRand.GetRandInRange(
      static_cast<uint32_t>(MIN_DIAMETER), static_cast<uint32_t>(MAX_DIAMETER + 1));

  for (auto& circleDiameter : m_circleDiameters)
  {
    circleDiameter = fixedCircleDiameter;
  }
}

auto CirclesFx::CirclesFxImpl::GetRandomColorMap() const -> const IColorMap&
{
  assert(m_colorMaps);
  return m_colorMaps->GetRandomColorMap(m_colorMaps->GetRandomGroup());
}

void CirclesFx::CirclesFxImpl::SetWeightedColorMaps(
    std::shared_ptr<COLOR::RandomColorMaps> weightedMaps)
{
  m_colorMaps = weightedMaps;

  ChangeCircleColorMaps();
  ChangeCircleDiameters();
}

inline void CirclesFx::CirclesFxImpl::ResetCentreTargetPosition()
{
  if ((!m_positionT.HasJustHitStartBoundary()) && (!m_positionT.HasJustHitEndBoundary()))
  {
    return;
  }

  constexpr int32_t MARGIN = 50;
  const Point2dInt randomPosition{
      m_goomRand.GetRandInRange(MARGIN, static_cast<int32_t>(m_draw.GetScreenWidth() - MARGIN)),
      m_goomRand.GetRandInRange(MARGIN, static_cast<int32_t>(m_draw.GetScreenHeight()) - MARGIN)};

  constexpr float TARGET_T = 0.8F;
  m_centreTargetPosition = lerp(m_zoomMidPoint, randomPosition, TARGET_T);
}

void CirclesFx::CirclesFxImpl::SetZoomMidPoint([[maybe_unused]] const Point2dInt& zoomMidPoint)
{
  m_zoomMidPoint = zoomMidPoint;
}

void CirclesFx::CirclesFxImpl::Start()
{
  ChangeCircleColorMaps();
}

inline void CirclesFx::CirclesFxImpl::ApplyMultiple()
{
  UpdateTime();

  UpdateSpeeds();
  DrawNextCircles();
}

inline void CirclesFx::CirclesFxImpl::UpdateTime()
{
  ++m_updateNum;
  m_blankTimer.Increment();
}

inline void CirclesFx::CirclesFxImpl::UpdateSpeeds()
{
  constexpr float PROB_CHANGE_SPEED = 0.2F;

  if (m_goomRand.ProbabilityOf(PROB_CHANGE_SPEED))
  {
    const uint32_t numSteps = std::min(
        MIN_POSITION_STEPS + m_goomInfo.GetSoundInfo().GetTimeSinceLastGoom(), MAX_POSITION_STEPS);
    m_positionT.SetNumSteps(numSteps);

    UpdateColorTs();
  }
}

void CirclesFx::CirclesFxImpl::UpdateColorTs()
{
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

inline void CirclesFx::CirclesFxImpl::DrawNextCircles()
{
  if (!m_blankTimer.Finished())
  {
    return;
  }

  DrawTheCircles();

  ResetCentreTargetPosition();
  m_circleRandomStartingPositions =
      GetRandomStartingCirclePositions(m_goomRand, static_cast<int32_t>(m_draw.GetScreenWidth()),
                                       static_cast<int32_t>(m_draw.GetScreenHeight()));

  UpdateTs();
}

void CirclesFx::CirclesFxImpl::DrawTheCircles()
{
  constexpr float BRIGHTNESS_CUT = 0.001F;
  const float brightness = m_positionT() < (1.0F - POSITION_MARGIN) ? 1.0F : BRIGHTNESS_CUT;

  constexpr uint32_t NUM_CIRCLES = NUM_CIRCLE_PATHS;
  const std::array<Point2dInt, NUM_CIRCLES> circleCentrePositions = GetCircleCentrePositions();

  const uint32_t fixedCircleDiameter = m_goomRand.GetRandInRange(
      static_cast<uint32_t>(MIN_DIAMETER + 2), static_cast<uint32_t>(MAX_DIAMETER + 1));

  Point2dInt prevCircleCentre = circleCentrePositions[NUM_CIRCLES - 1];
  const float circleBrightness = 0 == (m_updateNum % 2) ? (5.0F * brightness) : brightness;
  const float lineBrightness = 0 == (m_updateNum % 5) ? (10.0F * brightness) : brightness;
  const uint8_t lineThickness = (0 == (m_updateNum % 5)) ? 5 : 1;

  constexpr float T_LINE_COLOR_STEP = 1.0F / static_cast<float>(NUM_CIRCLE_PATHS);
  float tLineColor = 0.0F;
  for (size_t i = 0; i < NUM_CIRCLES; ++i)
  {
    const float tColor = m_colorTs.at(i)();
    const Pixel color = GetFinalColor(
        circleBrightness, 0 == (m_updateNum % 5) ? m_circleLowColorMaps.at(i)->GetColor(tColor)
                                                 : m_circleColorMaps.at(i)->GetColor(tColor));
    const Pixel lowColor =
        GetFinalColor(1.1F * circleBrightness, m_circleColorMaps.at(i)->GetColor(tColor));
    //GetFinalLowColor(circleBrightness, m_circleLowColorMaps.at(i)->GetColor(tColor));

    const Point2dInt circleCentre = circleCentrePositions.at(i);

    DrawCircle(circleCentre, fixedCircleDiameter, color, lowColor);

    const Pixel linesColor =
        GetFinalColor(lineBrightness, m_circleColorMaps.at(i)->GetColor(tLineColor));
    const Pixel linesLowColor =
        GetFinalLowColor(lineBrightness, m_circleLowColorMaps.at(i)->GetColor(tLineColor));
    DrawLine(prevCircleCentre, circleCentre, linesColor, linesLowColor, lineThickness);

    prevCircleCentre = circleCentre;
    tLineColor += T_LINE_COLOR_STEP;
  }
}

auto CirclesFx::CirclesFxImpl::GetCircleCentrePositions() const
    -> std::array<Point2dInt, NUM_CIRCLE_PATHS>
{
  const float tPosition =
      m_positionT() < (1.0F - POSITION_MARGIN) ? m_positionT() : (1.0F - POSITION_MARGIN);
  const Point2dInt circleTargetPosition = GetNextTargetPosition();

  std::array<Point2dInt, NUM_CIRCLE_PATHS> circleCentrePositions{};

  for (size_t i = 0; i < NUM_CIRCLE_PATHS; ++i)
  {
    //    const float newT = std::min(1.0F, tPosition + 0.75F);
    //    const V2dInt newCircleStartingPosition =
    //        lerp(m_circleRandomStartingPositions.at(i), m_circleStartingPositions.at(i), newT);

    const Point2dInt newCircleStartingPosition = m_circleStartingPositions.at(i);

    const Point2dInt jitter = {m_goomRand.GetRandInRange(-2, +3),
                               m_goomRand.GetRandInRange(-2, +3)};
    circleCentrePositions.at(i) =
        jitter + Vec2dInt{lerp(newCircleStartingPosition, circleTargetPosition, tPosition)};
  }

  return circleCentrePositions;
}

inline void CirclesFx::CirclesFxImpl::UpdateTs()
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

inline auto CirclesFx::CirclesFxImpl::GetNextTargetPosition() const -> Point2dInt
{
  return m_centreTargetPosition;
}

inline auto CirclesFx::CirclesFxImpl::GetFinalColor(const float brightness,
                                                    const Pixel& color) const -> Pixel
{
  constexpr float COLOR_BRIGHTNESS = 12.0F;
  return GetCorrectedColor(brightness * COLOR_BRIGHTNESS, color);
}

inline auto CirclesFx::CirclesFxImpl::GetFinalLowColor(const float brightness,
                                                       const Pixel& lowColor) const -> Pixel
{
  constexpr float LOW_COLOR_BRIGHTNESS = 15.0F;
  return GetCorrectedColor(brightness * LOW_COLOR_BRIGHTNESS, lowColor);
}

inline auto CirclesFx::CirclesFxImpl::GetCorrectedColor(const float brightness,
                                                        const Pixel& color) const -> Pixel
{
  return GetGammaCorrection(brightness, GetIncreasedChroma(color));
}

void CirclesFx::CirclesFxImpl::DrawCircle(const Point2dInt& centre,
                                          const uint32_t diameter,
                                          const Pixel& color,
                                          const Pixel& lowColor)
{
  const auto getColor1 =
      [&]([[maybe_unused]] const size_t x, [[maybe_unused]] const size_t y, const Pixel& bgnd)
  {
    if (0 == bgnd.A())
    {
      return Pixel::BLACK;
    }
    return color;
  };
  const auto getColor2 =
      [&]([[maybe_unused]] const size_t x, [[maybe_unused]] const size_t y, const Pixel& bgnd)
  {
    if (0 == bgnd.A())
    {
      return Pixel::BLACK;
    }
    return lowColor;
  };

  m_draw.Bitmap(centre.x, centre.y, GetImageBitmap(diameter), {getColor1, getColor2});
}

inline void CirclesFx::CirclesFxImpl::DrawLine(const Point2dInt& pos1,
                                               const Point2dInt& pos2,
                                               const Pixel& color,
                                               const Pixel& lowColor,
                                               [[maybe_unused]] const uint8_t lineThickness)
{
  constexpr uint32_t NUM_DOTS = 5;
  constexpr float T_STEP = 1.0F / static_cast<float>(NUM_DOTS);
  float t = T_STEP;
  for (uint32_t i = 0; i < (NUM_DOTS - 1); ++i)
  {
    const Point2dInt pos = lerp(pos1, pos2, t);
    if (0 == (i % 2))
    {
      DrawCircle(pos, 5, color, color);
    }
    else
    {
      DrawCircle(pos, 5, lowColor, lowColor);
    }
    t += T_STEP;
  }
  //m_draw.Line(pos1.x, pos1.y, pos2.x, pos2.y, {color, lowColor}, lineThickness);
}

inline auto CirclesFx::CirclesFxImpl::GetImageBitmap(const size_t size) const -> const ImageBitmap&
{
  return m_smallBitmaps.GetImageBitmap(m_currentBitmapName,
                                       std::clamp(size, MIN_DIAMETER, MAX_DIAMETER));
}

inline auto CirclesFx::CirclesFxImpl::GetGammaCorrection(const float brightness,
                                                         const Pixel& color) const -> Pixel
{
  if constexpr (1.0F == GAMMA)
  {
    return GetBrighterColor(brightness, color);
  }
  return m_gammaCorrect.GetCorrection(brightness, color);
}

} // namespace GOOM::VISUAL_FX
