#include "tube_fx.h"

#include "../draw/goom_draw.h"
#include "../draw/goom_draw_to_container.h"
#include "../draw/goom_draw_to_many.h"
#include "../tubes/tubes.h"
#include "goom_graphic.h"
#include "goom_plugin_info.h"
#include "goomutils/colorutils.h"
#include "goomutils/goomrand.h"
#include "goomutils/graphics/small_image_bitmaps.h"
#include "goomutils/logging_control.h"
//#undef NO_LOGGING
#include "goomutils/logging.h"
#include "goomutils/mathutils.h"
#include "goomutils/random_colormaps.h"
#include "goomutils/spimpl.h"
#include "goomutils/t_values.h"
#include "goomutils/timer.h"

#undef NDEBUG
#include <cassert>
#include <cstdint>
#include <memory>
#include <vector>

#if __cplusplus <= 201402L
namespace GOOM
{
namespace VISUAL_FX
{
#else
namespace GOOM::VISUAL_FX
{
#endif

using DRAW::GoomDrawToContainer;
using DRAW::GoomDrawToMany;
using DRAW::IGoomDraw;
using TUBES::BrightnessAttenuation;
using TUBES::PathParams;
using TUBES::Tube;
using UTILS::GetBrighterColor;
using UTILS::GetColorAverage;
using UTILS::GetRandInRange;
using UTILS::IColorMap;
using UTILS::ImageBitmap;
using UTILS::Logging;
using UTILS::ProbabilityOf;
using UTILS::RandomColorMaps;
using UTILS::SMALL_FLOAT;
using UTILS::SmallImageBitmaps;
using UTILS::Timer;
using UTILS::TValue;

constexpr size_t NUM_TUBES = 3;

struct TubeSettings
{
  bool noMoveFromCentre;
  bool noOscillating;
  float brightnessFactor;
  float radiusEdgeOffset;
  PathParams circlePathParams;
};
constexpr std::array<TubeSettings, NUM_TUBES> TUBE_SETTINGS{{
    {true, false, 2.4F, 150.0F, {10.0F, +0.5F, +0.5F}},
    {false, false, 0.19F, 130.0F, {50.0F, -0.75F, -1.0F}},
    {false, false, 0.18F, 130.0F, {40.0F, +1.0F, +0.75F}},
}};
constexpr size_t MAIN_TUBE_INDEX = 0;
constexpr size_t SECONDARY_TUBES_START_INDEX = 1;
constexpr PathParams COMMON_CIRCLE_PATH_PARMS{10.0F, +3.0F, +3.0F};

auto lerp(const PathParams& p0, const PathParams& p1, const float t) -> PathParams
{
  return {
      stdnew::lerp(p0.oscillatingAmplitude, p1.oscillatingAmplitude, t),
      stdnew::lerp(p0.xOscillatingFreq, p1.xOscillatingFreq, t),
      stdnew::lerp(p0.yOscillatingFreq, p1.yOscillatingFreq, t),
  };
}

constexpr uint32_t MIN_COLORMAP_TIME = 100;
constexpr uint32_t MAX_COLORMAP_TIME = 1000;

constexpr float MIN_BRIGHTNESS_FACTOR = 0.01F;
constexpr float MAX_BRIGHTNESS_FACTOR = 0.20F;

constexpr uint32_t MIN_JITTER_TIME = 50;
constexpr uint32_t MAX_JITTER_TIME = 500;
constexpr float MIN_SHAPE_JITTER_OFFSET = 10.0F;
constexpr float MAX_SHAPE_JITTER_OFFSET = 20.0F;

constexpr uint32_t MIN_DECREASED_SPEED_TIME = 100;
constexpr uint32_t MAX_DECREASED_SPEED_TIME = 500;
constexpr uint32_t MIN_INCREASED_SPEED_TIME = 100;
constexpr uint32_t MAX_INCREASED_SPEED_TIME = 500;
constexpr uint32_t MIN_NORMAL_SPEED_TIME = 20;
constexpr uint32_t MAX_NORMAL_SPEED_TIME = 50;

constexpr uint32_t MIN_STAY_IN_CENTRE_TIME = 1000;
constexpr uint32_t MAX_STAY_IN_CENTRE_TIME = 1000;
constexpr uint32_t MIN_STAY_AWAY_FROM_CENTRE_TIME = 100;
constexpr uint32_t MAX_STAY_AWAY_FROM_CENTRE_TIME = 100;

constexpr float PROB_RESET_COLOR_MAPS = 1.0F / 3.0F;
constexpr float PROB_ALLOW_OVEREXPOSED = 1.0F / 1000.0F;
constexpr float PROB_DECREASE_SPEED = 1.0F / 5.0F;
constexpr float PROB_INCREASE_SPEED = 1.0F / 2.0F;
constexpr float PROB_RANDOM_INCREASE_SPEED = 1.0F / 20.0F;
constexpr float PROB_NORMAL_SPEED = 1.0F / 20.0F;
constexpr float PROB_NO_SHAPE_JITTER = 0.8F;
constexpr float PROB_PREV_SHAPES_JITTER = 0.0F;
constexpr float PROB_OSCILLATING_SHAPE_PATH = 1.0F;
constexpr float PROB_MOVE_AWAY_FROM_CENTRE = 0.3F;

class TubeFx::TubeFxImpl
{
public:
  TubeFxImpl(const IGoomDraw& draw,
             const PluginInfo& goomInfo,
             const SmallImageBitmaps& smallBitmaps) noexcept;

  void SetWeightedColorMaps(std::shared_ptr<UTILS::RandomColorMaps> weightedMaps);
  void SetWeightedLowColorMaps(std::shared_ptr<UTILS::RandomColorMaps> weightedMaps);

  void Start();
  void Resume();
  void Suspend();

  void ApplyMultiple();

private:
  const IGoomDraw& m_draw;
  GoomDrawToContainer m_drawToContainer;
  const DRAW::GoomDrawToMany m_drawToMany;
  const PluginInfo& m_goomInfo;
  const SmallImageBitmaps& m_smallBitmaps;
  uint64_t m_updateNum = 0;
  std::shared_ptr<RandomColorMaps> m_colorMaps{};
  std::shared_ptr<RandomColorMaps> m_lowColorMaps{};
  bool m_allowOverexposed = false;
  bool m_allowMovingAwayFromCentre = false;
  bool m_oscillatingShapePath = ProbabilityOf(PROB_OSCILLATING_SHAPE_PATH);
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
  const V2dInt m_middlePos;
  Timer m_allStayInCentreTimer;
  Timer m_allStayAwayFromCentreTimer;
  void IncrementAllJoinCentreT();
  [[nodiscard]] auto GetTransformedCentrePoint(uint32_t tubeId, const V2dInt& centre) const
      -> V2dInt;

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

  void DrawLineToOne(
      int x1, int y1, int x2, int y2, const std::vector<Pixel>& colors, uint8_t thickness);
  void DrawCircleToOne(
      int x, int y, int radius, const std::vector<Pixel>& colors, uint8_t thickness);
  void DrawImageToOne(int x,
                      int y,
                      SmallImageBitmaps::ImageNames imageName,
                      uint32_t size,
                      const std::vector<Pixel>& colors);
  void DrawLineToMany(
      int x1, int y1, int x2, int y2, const std::vector<Pixel>& colors, uint8_t thickness);
  void DrawCircleToMany(
      int x, int y, int radius, const std::vector<Pixel>& colors, uint8_t thickness);
  void DrawImageToMany(int x,
                       int y,
                       SmallImageBitmaps::ImageNames imageName,
                       uint32_t size,
                       const std::vector<Pixel>& colors);
  [[nodiscard]] auto GetImageBitmap(SmallImageBitmaps::ImageNames imageName, size_t size)
      -> const ImageBitmap&;
  static auto GetSimpleColorFuncs(const std::vector<Pixel>& colors)
      -> std::vector<IGoomDraw::GetBitmapColorFunc>;
};

TubeFx::TubeFx(const IGoomDraw& draw,
               const PluginInfo& goomInfo,
               const SmallImageBitmaps& smallBitmaps) noexcept
  : m_fxImpl{spimpl::make_unique_impl<TubeFxImpl>(draw, goomInfo, smallBitmaps)}
{
}

void TubeFx::ApplyNoDraw()
{
}

void TubeFx::SetWeightedColorMaps(const std::shared_ptr<RandomColorMaps> weightedMaps)
{
  m_fxImpl->SetWeightedColorMaps(weightedMaps);
}

void TubeFx::SetWeightedLowColorMaps(const std::shared_ptr<RandomColorMaps> weightedMaps)
{
  m_fxImpl->SetWeightedLowColorMaps(weightedMaps);
}

void TubeFx::Start()
{
  m_fxImpl->Start();
}

void TubeFx::Resume()
{
  m_fxImpl->Resume();
}

void TubeFx::Suspend()
{
  m_fxImpl->Suspend();
}

void TubeFx::Finish()
{
}

auto TubeFx::GetFxName() const -> std::string
{
  return "tube";
}

void TubeFx::ApplyMultiple()
{
  m_fxImpl->ApplyMultiple();
}

TubeFx::TubeFxImpl::TubeFxImpl(const IGoomDraw& draw,
                               const PluginInfo& goomInfo,
                               const SmallImageBitmaps& smallBitmaps) noexcept
  : m_draw{draw},
    m_drawToContainer{draw.GetScreenWidth(), draw.GetScreenHeight()},
    m_drawToMany{draw.GetScreenWidth(), draw.GetScreenHeight(), {&draw, &m_drawToContainer}},
    m_goomInfo{goomInfo},
    m_smallBitmaps{smallBitmaps},
    m_prevShapesBrightnessAttenuation{draw.GetScreenWidth(), draw.GetScreenHeight(),
                                      PREV_SHAPES_CUTOFF_BRIGHTNESS},
    m_middlePos{0, 0},
    m_allStayInCentreTimer{1},
    m_allStayAwayFromCentreTimer{MAX_STAY_AWAY_FROM_CENTRE_TIME},
    m_colorMapTimer{GetRandInRange(MIN_COLORMAP_TIME, MAX_COLORMAP_TIME + 1)},
    m_changedSpeedTimer{1},
    m_jitterTimer{1}
{
}

void TubeFx::TubeFxImpl::Start()
{
  m_updateNum = 0;
  m_numCapturedPrevShapesGroups = 0;

  InitTubes();
}

void TubeFx::TubeFxImpl::Resume()
{
  m_numCapturedPrevShapesGroups = 0;

  m_oscillatingShapePath = ProbabilityOf(PROB_OSCILLATING_SHAPE_PATH);
  m_allowMovingAwayFromCentre = ProbabilityOf(PROB_MOVE_AWAY_FROM_CENTRE);

  ResetTubes();
}

void TubeFx::TubeFxImpl::Suspend()
{
}

inline auto TubeFx::TubeFxImpl::GetImageBitmap(const SmallImageBitmaps::ImageNames imageName,
                                               const size_t size) -> const ImageBitmap&
{
  return m_smallBitmaps.GetImageBitmap(
      imageName,
      stdnew::clamp(size, SmallImageBitmaps::MIN_IMAGE_SIZE, SmallImageBitmaps::MAX_IMAGE_SIZE));
}

inline void TubeFx::TubeFxImpl::SetWeightedColorMaps(
    const std::shared_ptr<UTILS::RandomColorMaps> weightedMaps)
{
  m_colorMaps = weightedMaps;

  for (auto& tube : m_tubes)
  {
    tube.SetWeightedColorMaps(m_colorMaps);
  }
}

inline void TubeFx::TubeFxImpl::SetWeightedLowColorMaps(
    const std::shared_ptr<UTILS::RandomColorMaps> weightedMaps)
{
  m_lowColorMaps = weightedMaps;

  for (auto& tube : m_tubes)
  {
    tube.SetWeightedLowColorMaps(m_lowColorMaps);
  }
}

void TubeFx::TubeFxImpl::InitTubes()
{
  assert(m_colorMaps != nullptr);
  assert(m_lowColorMaps != nullptr);

  const Tube::DrawFuncs drawToOneFuncs{
      [&](const int x1, const int y1, const int x2, const int y2, const std::vector<Pixel>& colors,
          const uint8_t thickness) { DrawLineToOne(x1, y1, x2, y2, colors, thickness); },
      [&](const int x, const int y, const int radius, const std::vector<Pixel>& colors,
          const uint8_t thickness) { DrawCircleToOne(x, y, radius, colors, thickness); },
      [&](const int x, const int y, const SmallImageBitmaps::ImageNames imageName,
          const uint32_t size,
          const std::vector<Pixel>& colors) { DrawImageToOne(x, y, imageName, size, colors); },
  };
  const Tube::DrawFuncs drawToManyFuncs{
      [&](const int x1, const int y1, const int x2, const int y2, const std::vector<Pixel>& colors,
          const uint8_t thickness) { DrawLineToMany(x1, y1, x2, y2, colors, thickness); },
      [&](const int x, const int y, const int radius, const std::vector<Pixel>& colors,
          const uint8_t thickness) { DrawCircleToMany(x, y, radius, colors, thickness); },
      [&](const int x, const int y, const SmallImageBitmaps::ImageNames imageName,
          const uint32_t size,
          const std::vector<Pixel>& colors) { DrawImageToMany(x, y, imageName, size, colors); },
  };

  m_tubes.emplace_back(MAIN_TUBE_INDEX, drawToManyFuncs, m_draw.GetScreenWidth(),
                       m_draw.GetScreenHeight(), m_colorMaps, m_lowColorMaps,
                       TUBE_SETTINGS.at(MAIN_TUBE_INDEX).radiusEdgeOffset,
                       TUBE_SETTINGS.at(MAIN_TUBE_INDEX).brightnessFactor);
  for (uint32_t i = SECONDARY_TUBES_START_INDEX; i < NUM_TUBES; ++i)
  {
    m_tubes.emplace_back(i, drawToOneFuncs, m_draw.GetScreenWidth(), m_draw.GetScreenHeight(),
                         m_colorMaps, m_lowColorMaps, TUBE_SETTINGS.at(i).radiusEdgeOffset,
                         TUBE_SETTINGS.at(i).brightnessFactor);
  }

  for (auto& tube : m_tubes)
  {
    tube.ResetColorMaps();
    tube.SetCircleSpeed(Tube::NORMAL_CIRCLE_SPEED);
    tube.SetMaxJitterOffset(0);
  }

  InitPaths();
}

void TubeFx::TubeFxImpl::DrawLineToOne(const int x1,
                                       const int y1,
                                       const int x2,
                                       const int y2,
                                       const std::vector<Pixel>& colors,
                                       const uint8_t thickness)
{
  m_draw.Line(x1, y1, x2, y2, colors, thickness);
}

void TubeFx::TubeFxImpl::DrawLineToMany(const int x1,
                                        const int y1,
                                        const int x2,
                                        const int y2,
                                        const std::vector<Pixel>& colors,
                                        const uint8_t thickness)
{
  //m_drawToContainer.Line(x1, y1, x2, y2, colors, thickness);
  m_drawToMany.Line(x1, y1, x2, y2, colors, thickness);
}

void TubeFx::TubeFxImpl::DrawCircleToOne(const int x,
                                         const int y,
                                         const int radius,
                                         const std::vector<Pixel>& colors,
                                         [[maybe_unused]] const uint8_t thickness)
{
  m_draw.Circle(x, y, radius, colors);
}

void TubeFx::TubeFxImpl::DrawCircleToMany(const int x,
                                          const int y,
                                          const int radius,
                                          const std::vector<Pixel>& colors,
                                          [[maybe_unused]] const uint8_t thickness)
{
  //m_drawToContainer.Circle(x, y, radius, colors);
  m_drawToMany.Circle(x, y, radius, colors);
}

void TubeFx::TubeFxImpl::DrawImageToOne(const int x,
                                        const int y,
                                        const SmallImageBitmaps::ImageNames imageName,
                                        const uint32_t size,
                                        const std::vector<Pixel>& colors)
{
  m_draw.Bitmap(x, y, GetImageBitmap(imageName, size), GetSimpleColorFuncs(colors),
                m_allowOverexposed);
}

void TubeFx::TubeFxImpl::DrawImageToMany(const int x,
                                         const int y,
                                         const SmallImageBitmaps::ImageNames imageName,
                                         const uint32_t size,
                                         const std::vector<Pixel>& colors)
{
  //m_drawToContainer.Bitmap(x, y, GetImageBitmap(imageName, size), GetSimpleColorFuncs(colors),
  //                         m_allowOverexposed);
  m_drawToMany.Bitmap(x, y, GetImageBitmap(imageName, size), GetSimpleColorFuncs(colors),
                      m_allowOverexposed);
}

inline auto TubeFx::TubeFxImpl::GetSimpleColorFuncs(const std::vector<Pixel>& colors)
    -> std::vector<IGoomDraw::GetBitmapColorFunc>
{
  const auto getColor1 = [&]([[maybe_unused]] const size_t x, [[maybe_unused]] const size_t y,
                             [[maybe_unused]] const Pixel& b) -> Pixel { return colors[0]; };
  const auto getColor2 = [&]([[maybe_unused]] const size_t x, [[maybe_unused]] const size_t y,
                             [[maybe_unused]] const Pixel& b) -> Pixel { return colors[1]; };
  return {getColor1, getColor2};
}

void TubeFx::TubeFxImpl::InitPaths()
{
  const auto transformCentre = [this](const uint32_t tubeId, const V2dInt& centre) {
    return this->GetTransformedCentrePoint(tubeId, centre);
  };
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

void TubeFx::TubeFxImpl::ResetTubes()
{
  //  m_drawToContainer.ClearAll();

  for (size_t i = 0; i < m_tubes.size(); ++i)
  {
    //    m_tubes[i].ResetPaths();
    if (!TUBE_SETTINGS.at(i).noOscillating)
    {
      m_tubes[i].SetAllowOscillatingCirclePaths(m_oscillatingShapePath);
    }
  }
}

void TubeFx::TubeFxImpl::ApplyMultiple()
{
  DoUpdates();

  DrawPreviousShapes();
  DrawShapes();
}

void TubeFx::TubeFxImpl::DoUpdates()
{
  ++m_updateNum;

  m_colorMapTimer.Increment();
  m_changedSpeedTimer.Increment();
  m_jitterTimer.Increment();

  UpdatePreviousShapesSettings();
  UpdateColorMaps();
  UpdateSpeeds();
}

void TubeFx::TubeFxImpl::UpdateColorMaps()
{
  m_allowOverexposed = ProbabilityOf(PROB_ALLOW_OVEREXPOSED);

  if (m_goomInfo.GetSoundInfo().GetTimeSinceLastGoom() >= 1)
  {
    return;
  }

  for (auto& tube : m_tubes)
  {
    if (m_colorMapTimer.Finished() && ProbabilityOf(PROB_RESET_COLOR_MAPS))
    {
      m_colorMapTimer.SetTimeLimit(GetRandInRange(MIN_COLORMAP_TIME, MAX_COLORMAP_TIME + 1));
      tube.ResetColorMaps();
      tube.SetBrightnessFactor(GetRandInRange(MIN_BRIGHTNESS_FACTOR, MAX_BRIGHTNESS_FACTOR));
    }
  }
}

void TubeFx::TubeFxImpl::UpdateSpeeds()
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

void TubeFx::TubeFxImpl::DrawShapes()
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

void TubeFx::TubeFxImpl::DrawPreviousShapes()
{
  if (0 == m_drawToContainer.GetNumChangedCoords())
  {
    return;
  }

  DrawCapturedPreviousShapesGroups();
}

void TubeFx::TubeFxImpl::DrawTubeCircles()
{
  for (auto& tube : m_tubes)
  {
    if (!tube.IsActive())
    {
      continue;
    }

    tube.DrawCircleOfShapes();
    //    tube.RotateShapeColorMaps();
  }
}

void TubeFx::TubeFxImpl::AdjustTubePaths()
{
  if (!m_allowMovingAwayFromCentre)
  {
    return;
  }

  for (size_t i = 0; i < NUM_TUBES; ++i)
  {
    m_tubes[i].SetCirclePathParams(
        lerp(TUBE_SETTINGS.at(i).circlePathParams, COMMON_CIRCLE_PATH_PARMS, m_allJoinCentreT()));
  }
}

void TubeFx::TubeFxImpl::DrawCapturedPreviousShapesGroups()
{
  const float brightnessAttenuation = GetApproxBrightnessAttenuation();
  using ColorsList = GoomDrawToContainer::ColorsList;

  m_drawToContainer.IterateChangedCoordsNewToOld(
      [&](const int32_t x, const int32_t y, const ColorsList& colorsList) {
        const int32_t jitterAmount =
            !m_prevShapesJitter
                ? 0
                : GetRandInRange(-PREV_SHAPES_JITTER_AMOUNT, PREV_SHAPES_JITTER_AMOUNT + 1);
        const int32_t newX = GetClipped(x + jitterAmount, m_draw.GetScreenWidth() - 1);
        const int32_t newY = GetClipped(y + jitterAmount, m_draw.GetScreenHeight() - 1);

        const Pixel avColor = GetAverageColor(colorsList);
        constexpr float BRIGHTNESS_FACTOR = 0.1F;
        const float brightness = BRIGHTNESS_FACTOR * brightnessAttenuation;
        const Pixel newColor0 = GetBrighterColor(brightness, avColor, m_allowOverexposed);

        // IMPORTANT - Best results come from putting color in second buffer.
        m_draw.DrawPixels(newX, newY, {Pixel::BLACK, newColor0});
      });
}

inline auto TubeFx::TubeFxImpl::GetAverageColor(const GoomDrawToContainer::ColorsList& colorsList)
    -> Pixel
{
  if (1 == colorsList.size())
  {
    return colorsList[0][0];
  }

  std::vector<Pixel> allColors{};
  for (const auto& col : colorsList)
  {
    allColors.emplace_back(col[0]);
  }
  return GetColorAverage(allColors);
}

inline auto TubeFx::TubeFxImpl::GetClipped(const int32_t val, const uint32_t maxVal) -> int32_t
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

auto TubeFx::TubeFxImpl::GetApproxBrightnessAttenuation() const -> float
{
  constexpr float MIN_BRIGHTNESS = 0.1F;
  const GoomDrawToContainer::Coords& firstCoords = m_drawToContainer.GetChangedCoordsList().front();
  return m_prevShapesBrightnessAttenuation.GetPositionBrightness({firstCoords.x, firstCoords.y},
                                                                 MIN_BRIGHTNESS);
}

void TubeFx::TubeFxImpl::UpdatePreviousShapesSettings()
{
  m_prevShapesJitter = ProbabilityOf(PROB_PREV_SHAPES_JITTER);
}

auto TubeFx::TubeFxImpl::GetTransformedCentrePoint(const uint32_t tubeId,
                                                   const V2dInt& centre) const -> V2dInt
{
  if ((!m_allowMovingAwayFromCentre) || TUBE_SETTINGS.at(tubeId).noMoveFromCentre)
  {
    return m_middlePos;
  }
  return lerp(centre, m_middlePos, m_allJoinCentreT());
}

void TubeFx::TubeFxImpl::IncrementAllJoinCentreT()
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
        GetRandInRange(MIN_STAY_IN_CENTRE_TIME, MAX_STAY_IN_CENTRE_TIME + 1));
  }
  else if (m_allJoinCentreT() <= (0.0F + SMALL_FLOAT))
  {
    m_allStayAwayFromCentreTimer.SetTimeLimit(
        GetRandInRange(MIN_STAY_AWAY_FROM_CENTRE_TIME, MAX_STAY_AWAY_FROM_CENTRE_TIME + 1));
  }

  m_allJoinCentreT.Increment();
}

void TubeFx::TubeFxImpl::ChangeSpeedForLowerVolumes(Tube& tube)
{
  if (ProbabilityOf(PROB_DECREASE_SPEED))
  {
    tube.DecreaseCentreSpeed();
    tube.DecreaseCircleSpeed();

    m_changedSpeedTimer.SetTimeLimit(
        GetRandInRange(MIN_DECREASED_SPEED_TIME, MAX_DECREASED_SPEED_TIME + 1));
  }
  else if (ProbabilityOf(PROB_NORMAL_SPEED))
  {
    tube.SetCentreSpeed(Tube::NORMAL_CENTRE_SPEED);
    tube.SetCircleSpeed(Tube::NORMAL_CIRCLE_SPEED);

    m_changedSpeedTimer.SetTimeLimit(
        GetRandInRange(MIN_NORMAL_SPEED_TIME, MAX_NORMAL_SPEED_TIME + 1));
  }
  else if (ProbabilityOf(PROB_RANDOM_INCREASE_SPEED))
  {
    tube.IncreaseCentreSpeed();
    tube.IncreaseCircleSpeed();

    m_changedSpeedTimer.SetTimeLimit(
        GetRandInRange(MIN_INCREASED_SPEED_TIME, MAX_INCREASED_SPEED_TIME + 1));
  }
}

void TubeFx::TubeFxImpl::ChangeSpeedForHigherVolumes(Tube& tube)
{
  if (ProbabilityOf(PROB_INCREASE_SPEED))
  {
    tube.IncreaseCentreSpeed();
    tube.IncreaseCircleSpeed();

    m_changedSpeedTimer.SetTimeLimit(
        GetRandInRange(MIN_INCREASED_SPEED_TIME, MAX_INCREASED_SPEED_TIME + 1));
  }
}

void TubeFx::TubeFxImpl::ChangeJitterOffsets(Tube& tube)
{
  if (ProbabilityOf(PROB_NO_SHAPE_JITTER))
  {
    tube.SetMaxJitterOffset(0);
  }
  else
  {
    const auto maxJitter = static_cast<int32_t>(std::round(
        stdnew::lerp(MIN_SHAPE_JITTER_OFFSET, MAX_SHAPE_JITTER_OFFSET, m_shapeJitterT())));
    tube.SetMaxJitterOffset(maxJitter);
    m_shapeJitterT.Increment();
    m_jitterTimer.SetTimeLimit(GetRandInRange(MIN_JITTER_TIME, MAX_JITTER_TIME + 1));
  }
}

#if __cplusplus <= 201402L
} // namespace VISUAL_FX
} // namespace GOOM
#else
} // namespace GOOM::VISUAL_FX
#endif
