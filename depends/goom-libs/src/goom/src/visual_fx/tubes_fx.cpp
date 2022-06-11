#include "tubes_fx.h"

//#undef NO_LOGGING

#include "color/colorutils.h"
#include "color/random_colormaps.h"
#include "draw/goom_draw.h"
#include "draw/goom_draw_to_container.h"
#include "draw/goom_draw_to_many.h"
#include "fx_helper.h"
#include "goom/logging.h"
#include "goom/spimpl.h"
#include "goom_config.h"
#include "goom_graphic.h"
#include "goom_plugin_info.h"
#include "point2d.h"
#include "tubes/tubes.h"
#include "utils/graphics/small_image_bitmaps.h"
#include "utils/math/goom_rand_base.h"
#include "utils/math/misc.h"
#include "utils/math/paths.h"
#include "utils/t_values.h"
#include "utils/timer.h"

#include <cstdint>
#include <memory>
#include <vector>

namespace GOOM::VISUAL_FX
{

using COLOR::GetBrighterColor;
using COLOR::GetColorAverage;
using COLOR::RandomColorMaps;
using DRAW::GoomDrawToContainer;
using DRAW::GoomDrawToMany;
using DRAW::IGoomDraw;
using DRAW::MultiplePixels;
using TUBES::BrightnessAttenuation;
using TUBES::Tube;
using TUBES::TubeData;
using TUBES::TubeDrawFuncs;
using UTILS::Logging;
using UTILS::Timer;
using UTILS::TValue;
using UTILS::GRAPHICS::ImageBitmap;
using UTILS::GRAPHICS::SmallImageBitmaps;
using UTILS::MATH::IGoomRand;
using UTILS::MATH::OscillatingFunction;
using UTILS::MATH::OscillatingPath;
using UTILS::MATH::SMALL_FLOAT;
using UTILS::MATH::U_HALF;

static constexpr size_t NUM_TUBES = 3;

struct TubeSettings
{
  bool noMoveFromCentre;
  bool noOscillating;
  float brightnessFactor;
  float radiusEdgeOffset;
  OscillatingFunction::Params circlePathParams;
};
static constexpr std::array<TubeSettings, NUM_TUBES> TUBE_SETTINGS{
    {
     {true, false, 3.4F, 150.0F, {10.0F, +0.5F, +0.5F}},
     {false, false, 0.19F, 130.0F, {50.0F, -0.75F, -1.0F}},
     {false, false, 0.18F, 130.0F, {40.0F, +1.0F, +0.75F}},
     }
};
static constexpr uint32_t MAIN_TUBE_INDEX = 0;
static constexpr uint32_t SECONDARY_TUBES_START_INDEX = 1;
static constexpr OscillatingFunction::Params COMMON_CIRCLE_PATH_PARAMS{10.0F, +3.0F, +3.0F};

[[nodiscard]] inline auto lerp(const OscillatingFunction::Params& params0,
                               const OscillatingFunction::Params& params1,
                               const float t) -> OscillatingFunction::Params
{
  return {
      STD20::lerp(params0.oscillatingAmplitude, params1.oscillatingAmplitude, t),
      STD20::lerp(params0.xOscillatingFreq, params1.xOscillatingFreq, t),
      STD20::lerp(params0.yOscillatingFreq, params1.yOscillatingFreq, t),
  };
}

static constexpr uint32_t MIN_COLORMAP_TIME = 100;
static constexpr uint32_t MAX_COLORMAP_TIME = 1000;

static constexpr float MIN_BRIGHTNESS_FACTOR = 0.01F;
static constexpr float MAX_BRIGHTNESS_FACTOR = 0.20F;

static constexpr uint32_t MIN_JITTER_TIME = 50;
static constexpr uint32_t MAX_JITTER_TIME = 500;
static constexpr float MIN_SHAPE_JITTER_OFFSET = 10.0F;
static constexpr float MAX_SHAPE_JITTER_OFFSET = 20.0F;

static constexpr uint32_t MIN_DECREASED_SPEED_TIME = 100;
static constexpr uint32_t MAX_DECREASED_SPEED_TIME = 500;
static constexpr uint32_t MIN_INCREASED_SPEED_TIME = 100;
static constexpr uint32_t MAX_INCREASED_SPEED_TIME = 500;
static constexpr uint32_t MIN_NORMAL_SPEED_TIME = 20;
static constexpr uint32_t MAX_NORMAL_SPEED_TIME = 50;

static constexpr uint32_t MIN_STAY_IN_CENTRE_TIME = 1000;
static constexpr uint32_t MAX_STAY_IN_CENTRE_TIME = 1000;
static constexpr uint32_t MIN_STAY_AWAY_FROM_CENTRE_TIME = 100;
static constexpr uint32_t MAX_STAY_AWAY_FROM_CENTRE_TIME = 100;

static constexpr float PROB_RESET_COLOR_MAPS = 1.0F / 3.0F;
static constexpr float PROB_DECREASE_SPEED = 1.0F / 5.0F;
static constexpr float PROB_INCREASE_SPEED = 1.0F / 2.0F;
static constexpr float PROB_RANDOM_INCREASE_SPEED = 1.0F / 20.0F;
static constexpr float PROB_NORMAL_SPEED = 1.0F / 20.0F;
static constexpr float PROB_NO_SHAPE_JITTER = 0.8F;
static constexpr float PROB_PREV_SHAPES_JITTER = 0.0F;
static constexpr float PROB_OSCILLATING_SHAPE_PATH = 1.0F;
static constexpr float PROB_MOVE_AWAY_FROM_CENTRE = 0.3F;
static constexpr float PROB_FOLLOW_ZOOM_MID_POINT = 0.3F;

class TubesFx::TubeFxImpl
{
public:
  TubeFxImpl(const FxHelper& fxHelper, const SmallImageBitmaps& smallBitmaps) noexcept;

  [[nodiscard]] auto GetCurrentColorMapsNames() const noexcept -> std::vector<std::string>;
  auto SetWeightedColorMaps(const WeightedColorMaps& weightedColorMaps) noexcept -> void;

  void SetZoomMidpoint(const Point2dInt& zoomMidpoint);

  void Start();
  void Resume();

  void ApplyMultiple();

private:
  IGoomDraw& m_draw;
  GoomDrawToContainer m_drawToContainer;
  GoomDrawToMany m_drawToMany;
  const PluginInfo& m_goomInfo;
  const IGoomRand& m_goomRand;
  const SmallImageBitmaps& m_smallBitmaps;
  uint64_t m_updateNum = 0;
  std::shared_ptr<RandomColorMaps> m_mainColorMaps{};
  std::shared_ptr<RandomColorMaps> m_lowColorMaps{};
  bool m_allowMovingAwayFromCentre = false;
  bool m_oscillatingShapePath;
  uint32_t m_numCapturedPrevShapesGroups = 0;
  static constexpr float PREV_SHAPES_CUTOFF_BRIGHTNESS = 0.005F;
  const BrightnessAttenuation m_prevShapesBrightnessAttenuation;
  [[nodiscard]] auto GetApproxBrightnessAttenuation() const -> float;
  bool m_prevShapesJitter = false;
  static constexpr int32_t PREV_SHAPES_JITTER_AMOUNT = 2;
  static constexpr uint32_t MIN_CAPTURED_PREV_SHAPES_GROUPS = 4;

  std::vector<Tube> m_tubes{};
  static constexpr float ALL_JOIN_CENTRE_STEP = 0.001F;
  TValue m_allJoinCentreT{TValue::StepType::CONTINUOUS_REVERSIBLE, ALL_JOIN_CENTRE_STEP};
  const Point2dInt m_screenMidpoint;
  Point2dInt m_targetMiddlePos{0, 0};
  Point2dInt m_previousMiddlePos{0, 0};
  static constexpr uint32_t MIDDLE_POS_NUM_STEPS = 100;
  TValue m_middlePosT{TValue::StepType::SINGLE_CYCLE, MIDDLE_POS_NUM_STEPS, TValue::MAX_T_VALUE};
  [[nodiscard]] auto GetMiddlePos() const -> Point2dInt;
  Timer m_allStayInCentreTimer;
  Timer m_allStayAwayFromCentreTimer;
  void IncrementAllJoinCentreT();
  [[nodiscard]] auto GetTransformedCentreVector(const uint32_t tubeId,
                                                const Point2dInt& centre) const -> Vec2dInt;

  static constexpr float JITTER_STEP = 0.1F;
  TValue m_shapeJitterT{TValue::StepType::CONTINUOUS_REVERSIBLE, JITTER_STEP};

  Timer m_colorMapTimer;
  Timer m_changedSpeedTimer;
  Timer m_jitterTimer;
  void InitTubes();
  void InitPaths();
  void ResetTubes();
  void DoUpdates();
  void DrawShapes();
  void AdjustTubePaths();
  void DrawTubeCircles();
  void DrawPreviousShapes();
  void DrawCapturedPreviousShapesGroups();
  [[nodiscard]] static auto GetAverageColor(const GoomDrawToContainer::ColorsList& colorsList)
      -> Pixel;
  [[nodiscard]] static auto GetClipped(int32_t val, uint32_t maxVal) -> int32_t;
  void UpdatePreviousShapesSettings();
  void UpdateColorMaps();
  void UpdateSpeeds();
  void ChangeSpeedForLowerVolumes(Tube& tube);
  void ChangeSpeedForHigherVolumes(Tube& tube);
  void ChangeJitterOffsets(Tube& tube);

  void DrawLineToOne(Point2dInt point1,
                     Point2dInt point2,
                     const MultiplePixels& colors,
                     uint8_t thickness);
  void DrawCircleToOne(Point2dInt point,
                       int radius,
                       const MultiplePixels& colors,
                       uint8_t thickness);
  void DrawImageToOne(Point2dInt point,
                      SmallImageBitmaps::ImageNames imageName,
                      uint32_t size,
                      const MultiplePixels& colors);
  void DrawLineToMany(Point2dInt point1,
                      Point2dInt point2,
                      const MultiplePixels& colors,
                      uint8_t thickness);
  void DrawCircleToMany(Point2dInt point,
                        int radius,
                        const MultiplePixels& colors,
                        uint8_t thickness);
  void DrawImageToMany(Point2dInt point,
                       SmallImageBitmaps::ImageNames imageName,
                       uint32_t size,
                       const MultiplePixels& colors);
  [[nodiscard]] auto GetImageBitmap(SmallImageBitmaps::ImageNames imageName, size_t size) const
      -> const ImageBitmap&;
  static auto GetSimpleColorFuncs(const MultiplePixels& colors)
      -> std::vector<IGoomDraw::GetBitmapColorFunc>;
};

TubesFx::TubesFx(const FxHelper& fxHelper, const SmallImageBitmaps& smallBitmaps) noexcept
  : m_pimpl{spimpl::make_unique_impl<TubeFxImpl>(fxHelper, smallBitmaps)}
{
}

auto TubesFx::GetFxName() const noexcept -> std::string
{
  return "tube";
}

auto TubesFx::GetCurrentColorMapsNames() const noexcept -> std::vector<std::string>
{
  return m_pimpl->GetCurrentColorMapsNames();
}

auto TubesFx::SetWeightedColorMaps(const WeightedColorMaps& weightedColorMaps) noexcept -> void
{
  m_pimpl->SetWeightedColorMaps(weightedColorMaps);
}

auto TubesFx::SetZoomMidpoint(const Point2dInt& zoomMidpoint) noexcept -> void
{
  m_pimpl->SetZoomMidpoint(zoomMidpoint);
}

auto TubesFx::Start() noexcept -> void
{
  m_pimpl->Start();
}

auto TubesFx::Finish() noexcept -> void
{
  // Not needed.
}

auto TubesFx::Resume() noexcept -> void
{
  m_pimpl->Resume();
}

auto TubesFx::Suspend() noexcept -> void
{
  // Not needed.
}

auto TubesFx::ApplyMultiple() noexcept -> void
{
  m_pimpl->ApplyMultiple();
}

TubesFx::TubeFxImpl::TubeFxImpl(const FxHelper& fxHelper,
                               const SmallImageBitmaps& smallBitmaps) noexcept
  : m_draw{fxHelper.GetDraw()},
    m_drawToContainer{m_draw.GetScreenWidth(), m_draw.GetScreenHeight()},
    m_drawToMany{m_draw.GetScreenWidth(), m_draw.GetScreenHeight(), {&m_draw, &m_drawToContainer}},
    m_goomInfo{fxHelper.GetGoomInfo()},
    m_goomRand{fxHelper.GetGoomRand()},
    m_smallBitmaps{smallBitmaps},
    m_oscillatingShapePath{m_goomRand.ProbabilityOf(PROB_OSCILLATING_SHAPE_PATH)},
    m_prevShapesBrightnessAttenuation{m_draw.GetScreenWidth(), m_draw.GetScreenHeight(),
                                      PREV_SHAPES_CUTOFF_BRIGHTNESS},
    m_screenMidpoint{U_HALF * m_draw.GetScreenWidth(), U_HALF * m_draw.GetScreenHeight()},
    m_allStayInCentreTimer{1},
    m_allStayAwayFromCentreTimer{MAX_STAY_AWAY_FROM_CENTRE_TIME},
    m_colorMapTimer{m_goomRand.GetRandInRange(MIN_COLORMAP_TIME, MAX_COLORMAP_TIME + 1)},
    m_changedSpeedTimer{1},
    m_jitterTimer{1}
{
}

void TubesFx::TubeFxImpl::Start()
{
  m_updateNum = 0;
  m_numCapturedPrevShapesGroups = 0;

  InitTubes();
}

void TubesFx::TubeFxImpl::Resume()
{
  m_numCapturedPrevShapesGroups = 0;

  m_oscillatingShapePath = m_goomRand.ProbabilityOf(PROB_OSCILLATING_SHAPE_PATH);
  m_allowMovingAwayFromCentre = m_goomRand.ProbabilityOf(PROB_MOVE_AWAY_FROM_CENTRE);

  ResetTubes();
}

inline auto TubesFx::TubeFxImpl::GetImageBitmap(const SmallImageBitmaps::ImageNames imageName,
                                                const size_t size) const -> const ImageBitmap&
{
  return m_smallBitmaps.GetImageBitmap(
      imageName,
      std::clamp(size, SmallImageBitmaps::MIN_IMAGE_SIZE, SmallImageBitmaps::MAX_IMAGE_SIZE));
}

auto TubesFx::TubeFxImpl::GetCurrentColorMapsNames() const noexcept -> std::vector<std::string>
{
  return {m_mainColorMaps->GetColorMapsName(), m_lowColorMaps->GetColorMapsName()};
}

auto TubesFx::TubeFxImpl::SetWeightedColorMaps(const WeightedColorMaps& weightedColorMaps) noexcept
    -> void
{
  Expects(weightedColorMaps.mainColorMaps != nullptr);
  m_mainColorMaps = weightedColorMaps.mainColorMaps;
  for (auto& tube : m_tubes)
  {
    tube.SetWeightedMainColorMaps(m_mainColorMaps);
  }

  Expects(weightedColorMaps.lowColorMaps != nullptr);
  m_lowColorMaps = weightedColorMaps.lowColorMaps;
  for (auto& tube : m_tubes)
  {
    tube.SetWeightedLowColorMaps(m_lowColorMaps);
  }
}

inline void TubesFx::TubeFxImpl::SetZoomMidpoint(const Point2dInt& zoomMidpoint)
{
  m_previousMiddlePos = GetMiddlePos();
  m_middlePosT.Reset();

  if (m_goomRand.ProbabilityOf(PROB_FOLLOW_ZOOM_MID_POINT))
  {
    m_targetMiddlePos = zoomMidpoint - Vec2dInt{m_screenMidpoint};
  }
  else
  {
    m_targetMiddlePos = {0, 0};
  }
}

inline auto TubesFx::TubeFxImpl::GetMiddlePos() const -> Point2dInt
{
  return lerp(m_previousMiddlePos, m_targetMiddlePos, m_middlePosT());
}

void TubesFx::TubeFxImpl::InitTubes()
{
  Expects(m_mainColorMaps != nullptr);
  Expects(m_lowColorMaps != nullptr);

  const TubeDrawFuncs drawToOneFuncs{
      [this](const Point2dInt point1, const Point2dInt point2, const MultiplePixels& colors,
             const uint8_t thickness) { DrawLineToOne(point1, point2, colors, thickness); },
      [this](const Point2dInt point, const int radius, const MultiplePixels& colors,
             const uint8_t thickness) { DrawCircleToOne(point, radius, colors, thickness); },
      [this](const Point2dInt point, const SmallImageBitmaps::ImageNames imageName,
             const uint32_t size, const MultiplePixels& colors)
      { DrawImageToOne(point, imageName, size, colors); },
  };
  const TubeDrawFuncs drawToManyFuncs{
      [this](const Point2dInt point1, const Point2dInt point2, const MultiplePixels& colors,
             const uint8_t thickness) { DrawLineToMany(point1, point2, colors, thickness); },
      [this](const Point2dInt point, const int radius, const MultiplePixels& colors,
             const uint8_t thickness) { DrawCircleToMany(point, radius, colors, thickness); },
      [this](const Point2dInt point, const SmallImageBitmaps::ImageNames imageName,
             const uint32_t size, const MultiplePixels& colors)
      { DrawImageToMany(point, imageName, size, colors); },
  };

  const TubeData mainTubeData{MAIN_TUBE_INDEX,
                              drawToManyFuncs,
                              m_draw.GetScreenWidth(),
                              m_draw.GetScreenHeight(),
                              m_goomRand,
                              m_mainColorMaps,
                              m_lowColorMaps,
                              TUBE_SETTINGS.at(MAIN_TUBE_INDEX).radiusEdgeOffset,
                              TUBE_SETTINGS.at(MAIN_TUBE_INDEX).brightnessFactor};
  m_tubes.emplace_back(mainTubeData, TUBE_SETTINGS.at(MAIN_TUBE_INDEX).circlePathParams);

  for (uint32_t i = SECONDARY_TUBES_START_INDEX; i < NUM_TUBES; ++i)
  {
    const TubeData tubeData{i,
                            drawToOneFuncs,
                            m_draw.GetScreenWidth(),
                            m_draw.GetScreenHeight(),
                            m_goomRand,
                            m_mainColorMaps,
                            m_lowColorMaps,
                            TUBE_SETTINGS.at(i).radiusEdgeOffset,
                            TUBE_SETTINGS.at(i).brightnessFactor};
    m_tubes.emplace_back(tubeData, TUBE_SETTINGS.at(i).circlePathParams);
  }

  for (auto& tube : m_tubes)
  {
    tube.ResetColorMaps();
    tube.SetCircleSpeed(Tube::NORMAL_CIRCLE_SPEED);
    tube.SetMaxJitterOffset(0);
  }

  InitPaths();
}

inline void TubesFx::TubeFxImpl::DrawLineToOne(const Point2dInt point1,
                                               const Point2dInt point2,
                                               const MultiplePixels& colors,
                                               const uint8_t thickness)
{
  m_draw.Line(point1, point2, colors, thickness);
}

inline void TubesFx::TubeFxImpl::DrawLineToMany(const Point2dInt point1,
                                                const Point2dInt point2,
                                                const MultiplePixels& colors,
                                                const uint8_t thickness)
{
  m_drawToMany.Line(point1, point2, colors, thickness);
}

inline void TubesFx::TubeFxImpl::DrawCircleToOne(const Point2dInt point,
                                                 const int radius,
                                                 const MultiplePixels& colors,
                                                 [[maybe_unused]] const uint8_t thickness)
{
  m_draw.Circle(point, radius, colors);
}

inline void TubesFx::TubeFxImpl::DrawCircleToMany(const Point2dInt point,
                                                  const int radius,
                                                  const MultiplePixels& colors,
                                                  [[maybe_unused]] const uint8_t thickness)
{
  m_drawToMany.Circle(point, radius, colors);
}

inline void TubesFx::TubeFxImpl::DrawImageToOne(const Point2dInt point,
                                                const SmallImageBitmaps::ImageNames imageName,
                                                const uint32_t size,
                                                const MultiplePixels& colors)
{
  m_draw.Bitmap(point, GetImageBitmap(imageName, static_cast<size_t>(size)),
                GetSimpleColorFuncs(colors));
}

inline void TubesFx::TubeFxImpl::DrawImageToMany(const Point2dInt point,
                                                 const SmallImageBitmaps::ImageNames imageName,
                                                 const uint32_t size,
                                                 const MultiplePixels& colors)
{
  //m_drawToContainer.Bitmap(x, y, GetImageBitmap(imageName, size), GetSimpleColorFuncs(colors));
  m_drawToMany.Bitmap(point, GetImageBitmap(imageName, static_cast<size_t>(size)),
                      GetSimpleColorFuncs(colors));
}

inline auto TubesFx::TubeFxImpl::GetSimpleColorFuncs(const MultiplePixels& colors)
    -> std::vector<IGoomDraw::GetBitmapColorFunc>
{
  const auto getColor1 = [&colors]([[maybe_unused]] const size_t x, [[maybe_unused]] const size_t y,
                                   [[maybe_unused]] const Pixel& bgnd) { return colors[0]; };
  const auto getColor2 = [&colors]([[maybe_unused]] const size_t x, [[maybe_unused]] const size_t y,
                                   [[maybe_unused]] const Pixel& bgnd) { return colors[1]; };
  return {getColor1, getColor2};
}

void TubesFx::TubeFxImpl::InitPaths()
{
  const auto transformCentre = [this](const uint32_t tubeId, const Point2dInt& centre)
  { return this->GetTransformedCentreVector(tubeId, centre); };
  const float centreStep = 1.0F / static_cast<float>(m_tubes.size());
  float centreT = 0.0;
  for (auto& tube : m_tubes)
  {
    tube.SetCentrePathT(centreT);
    tube.SetTransformCentreFunc(transformCentre);
    centreT += centreStep;
  }

  for (size_t i = 0; i < NUM_TUBES; ++i)
  {
    m_tubes[i].SetCirclePathParams(TUBE_SETTINGS.at(i).circlePathParams);
  }
}

void TubesFx::TubeFxImpl::ResetTubes()
{
  //  m_drawToContainer.ClearAll();

  for (size_t i = 0; i < m_tubes.size(); ++i)
  {
    if (!TUBE_SETTINGS.at(i).noOscillating)
    {
      m_tubes[i].SetAllowOscillatingCirclePaths(m_oscillatingShapePath);
    }
  }
}

void TubesFx::TubeFxImpl::ApplyMultiple()
{
  DoUpdates();

  DrawPreviousShapes();
  DrawShapes();
}

void TubesFx::TubeFxImpl::DoUpdates()
{
  ++m_updateNum;

  m_colorMapTimer.Increment();
  m_changedSpeedTimer.Increment();
  m_jitterTimer.Increment();
  m_middlePosT.Increment();

  UpdatePreviousShapesSettings();
  UpdateColorMaps();
  UpdateSpeeds();
}

void TubesFx::TubeFxImpl::UpdateColorMaps()
{
  if (m_goomInfo.GetSoundInfo().GetTimeSinceLastGoom() >= 1)
  {
    return;
  }

  for (auto& tube : m_tubes)
  {
    if (m_colorMapTimer.Finished() && m_goomRand.ProbabilityOf(PROB_RESET_COLOR_MAPS))
    {
      m_colorMapTimer.SetTimeLimit(
          m_goomRand.GetRandInRange(MIN_COLORMAP_TIME, MAX_COLORMAP_TIME + 1));
      tube.ResetColorMaps();
      tube.SetBrightnessFactor(
          m_goomRand.GetRandInRange(MIN_BRIGHTNESS_FACTOR, MAX_BRIGHTNESS_FACTOR));
    }
  }
}

void TubesFx::TubeFxImpl::UpdateSpeeds()
{
  for (auto& tube : m_tubes)
  {
    if (!tube.IsActive())
    {
      continue;
    }

    if (m_jitterTimer.Finished())
    {
      tube.SetMaxJitterOffset(0);
    }

    if (m_changedSpeedTimer.Finished())
    {
      ChangeJitterOffsets(tube);

      if (m_goomInfo.GetSoundInfo().GetTimeSinceLastGoom() >= 1)
      {
        ChangeSpeedForLowerVolumes(tube);
      }
      else
      {
        ChangeSpeedForHigherVolumes(tube);
      }
    }
  }
}

void TubesFx::TubeFxImpl::DrawShapes()
{
  const size_t prevShapesSize = m_drawToContainer.GetNumChangedCoords();

  DrawTubeCircles();
  AdjustTubePaths();

  ++m_numCapturedPrevShapesGroups;
  if (m_numCapturedPrevShapesGroups >= MIN_CAPTURED_PREV_SHAPES_GROUPS)
  {
    m_drawToContainer.ResizeChangedCoordsKeepingNewest(prevShapesSize);
  }

  IncrementAllJoinCentreT();
}

void TubesFx::TubeFxImpl::DrawPreviousShapes()
{
  if (0 == m_drawToContainer.GetNumChangedCoords())
  {
    return;
  }

  DrawCapturedPreviousShapesGroups();
}

void TubesFx::TubeFxImpl::DrawTubeCircles()
{
  const auto drawTubeCircles = [&](const size_t i)
  {
    if (!m_tubes[i].IsActive())
    {
      return;
    }
    m_tubes[i].DrawCircleOfShapes();
    //    tube.RotateShapeColorMaps();
  };

  for (size_t i = 0; i < m_tubes.size(); ++i)
  {
    drawTubeCircles(i);
  }
}

void TubesFx::TubeFxImpl::AdjustTubePaths()
{
  if (!m_allowMovingAwayFromCentre)
  {
    return;
  }

  for (size_t i = 0; i < NUM_TUBES; ++i)
  {
    m_tubes[i].SetCirclePathParams(
        lerp(TUBE_SETTINGS.at(i).circlePathParams, COMMON_CIRCLE_PATH_PARAMS, m_allJoinCentreT()));
  }
}

void TubesFx::TubeFxImpl::DrawCapturedPreviousShapesGroups()
{
  const float brightnessAttenuation = GetApproxBrightnessAttenuation();
  using ColorsList = GoomDrawToContainer::ColorsList;

  m_drawToContainer.IterateChangedCoordsNewToOld(
      [this, &brightnessAttenuation](const Point2dInt point, const ColorsList& colorsList)
      {
        const int32_t jitterAmount = !m_prevShapesJitter
                                         ? 0
                                         : m_goomRand.GetRandInRange(-PREV_SHAPES_JITTER_AMOUNT,
                                                                     PREV_SHAPES_JITTER_AMOUNT + 1);
        const Point2dInt newPoint = {
            GetClipped(point.x + jitterAmount, m_draw.GetScreenWidth() - 1),
            GetClipped(point.y + jitterAmount, m_draw.GetScreenHeight() - 1)};

        const Pixel avColor = GetAverageColor(colorsList);
        static constexpr float BRIGHTNESS_FACTOR = 0.1F;
        const float brightness = BRIGHTNESS_FACTOR * brightnessAttenuation;
        const Pixel newColor0 = GetBrighterColor(brightness, avColor);

        // IMPORTANT - Best results come from putting color in second buffer.
        m_draw.DrawPixels(newPoint, {BLACK_PIXEL, newColor0});
      });
}

inline auto TubesFx::TubeFxImpl::GetAverageColor(const GoomDrawToContainer::ColorsList& colorsList)
    -> Pixel
{
  if (1 == colorsList.count)
  {
    return colorsList.colorsArray[0];
  }
  if (0 == colorsList.count)
  {
    return BLACK_PIXEL;
  }

  return GetColorAverage(colorsList.count, colorsList.colorsArray);
}

inline auto TubesFx::TubeFxImpl::GetClipped(const int32_t val, const uint32_t maxVal) -> int32_t
{
  if (val < 0)
  {
    return 0;
  }
  if (val > static_cast<int32_t>(maxVal))
  {
    return static_cast<int32_t>(maxVal);
  }
  return val;
}

auto TubesFx::TubeFxImpl::GetApproxBrightnessAttenuation() const -> float
{
  static constexpr float MIN_BRIGHTNESS = 0.1F;
  const Point2dInt& firstCoords = m_drawToContainer.GetChangedCoordsList().front();
  return m_prevShapesBrightnessAttenuation.GetPositionBrightness({firstCoords.x, firstCoords.y},
                                                                 MIN_BRIGHTNESS);
}

void TubesFx::TubeFxImpl::UpdatePreviousShapesSettings()
{
  m_prevShapesJitter = m_goomRand.ProbabilityOf(PROB_PREV_SHAPES_JITTER);
}

auto TubesFx::TubeFxImpl::GetTransformedCentreVector(const uint32_t tubeId,
                                                     const Point2dInt& centre) const -> Vec2dInt
{
  if ((!m_allowMovingAwayFromCentre) || TUBE_SETTINGS.at(tubeId).noMoveFromCentre)
  {
    return Vec2dInt{GetMiddlePos()};
  }
  return Vec2dInt{lerp(centre, GetMiddlePos(), m_allJoinCentreT())};
}

void TubesFx::TubeFxImpl::IncrementAllJoinCentreT()
{
  m_allStayInCentreTimer.Increment();
  if (!m_allStayInCentreTimer.Finished())
  {
    return;
  }
  m_allStayAwayFromCentreTimer.Increment();
  if (!m_allStayAwayFromCentreTimer.Finished())
  {
    return;
  }

  if (m_allJoinCentreT() >= (1.0F - SMALL_FLOAT))
  {
    m_allStayInCentreTimer.SetTimeLimit(
        m_goomRand.GetRandInRange(MIN_STAY_IN_CENTRE_TIME, MAX_STAY_IN_CENTRE_TIME + 1));
  }
  else if (m_allJoinCentreT() <= (0.0F + SMALL_FLOAT))
  {
    m_allStayAwayFromCentreTimer.SetTimeLimit(m_goomRand.GetRandInRange(
        MIN_STAY_AWAY_FROM_CENTRE_TIME, MAX_STAY_AWAY_FROM_CENTRE_TIME + 1));
  }

  m_allJoinCentreT.Increment();
}

void TubesFx::TubeFxImpl::ChangeSpeedForLowerVolumes(Tube& tube)
{
  if (m_goomRand.ProbabilityOf(PROB_DECREASE_SPEED))
  {
    tube.DecreaseCentreSpeed();
    tube.DecreaseCircleSpeed();

    m_changedSpeedTimer.SetTimeLimit(
        m_goomRand.GetRandInRange(MIN_DECREASED_SPEED_TIME, MAX_DECREASED_SPEED_TIME + 1));
  }
  else if (m_goomRand.ProbabilityOf(PROB_NORMAL_SPEED))
  {
    tube.SetCentreSpeed(Tube::NORMAL_CENTRE_SPEED);
    tube.SetCircleSpeed(Tube::NORMAL_CIRCLE_SPEED);

    m_changedSpeedTimer.SetTimeLimit(
        m_goomRand.GetRandInRange(MIN_NORMAL_SPEED_TIME, MAX_NORMAL_SPEED_TIME + 1));
  }
  else if (m_goomRand.ProbabilityOf(PROB_RANDOM_INCREASE_SPEED))
  {
    tube.IncreaseCentreSpeed();
    tube.IncreaseCircleSpeed();

    m_changedSpeedTimer.SetTimeLimit(
        m_goomRand.GetRandInRange(MIN_INCREASED_SPEED_TIME, MAX_INCREASED_SPEED_TIME + 1));
  }
}

void TubesFx::TubeFxImpl::ChangeSpeedForHigherVolumes(Tube& tube)
{
  if (m_goomRand.ProbabilityOf(PROB_INCREASE_SPEED))
  {
    tube.IncreaseCentreSpeed();
    tube.IncreaseCircleSpeed();

    m_changedSpeedTimer.SetTimeLimit(
        m_goomRand.GetRandInRange(MIN_INCREASED_SPEED_TIME, MAX_INCREASED_SPEED_TIME + 1));
  }
}

void TubesFx::TubeFxImpl::ChangeJitterOffsets(Tube& tube)
{
  if (m_goomRand.ProbabilityOf(PROB_NO_SHAPE_JITTER))
  {
    tube.SetMaxJitterOffset(0);
  }
  else
  {
    const auto maxJitter = static_cast<int32_t>(std::round(
        STD20::lerp(MIN_SHAPE_JITTER_OFFSET, MAX_SHAPE_JITTER_OFFSET, m_shapeJitterT())));
    tube.SetMaxJitterOffset(maxJitter);
    m_shapeJitterT.Increment();
    m_jitterTimer.SetTimeLimit(m_goomRand.GetRandInRange(MIN_JITTER_TIME, MAX_JITTER_TIME + 1));
  }
}

} // namespace GOOM::VISUAL_FX
