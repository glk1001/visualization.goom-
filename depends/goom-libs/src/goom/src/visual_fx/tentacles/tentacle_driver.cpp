//#undef NO_LOGGING

#include "tentacle_driver.h"

#include "color/color_maps.h"
#include "color/random_color_maps.h"
#include "draw/goom_draw.h"
#include "goom_config.h"
#include "goom_logger.h"
#include "tentacle3d.h"
#include "utils/debugging_logger.h"
#include "utils/math/goom_rand_base.h"
#include "utils/t_values.h"

#include <algorithm>
#include <array>
#include <memory>
#include <utility>
#include <vector>

namespace GOOM::VISUAL_FX::TENTACLES
{

using COLOR::ColorMaps;
using COLOR::ColorMapSharedPtrWrapper;
using COLOR::IColorMap;
using DRAW::GetLowColor;
using DRAW::GetMainColor;
using DRAW::IGoomDraw;
using DRAW::MultiplePixels;
using UTILS::IncrementedValue;
using UTILS::TValue;
using UTILS::MATH::IGoomRand;
using UTILS::MATH::SineWaveMultiplier;

namespace
{

constexpr auto MAIN_BRIGHTNESS_FACTOR = 0.5F;
constexpr auto LOW_BRIGHTNESS_FACTOR  = 1.0F;
constexpr auto PROB_LOW_MIX_SAME      = 0.5F;

constexpr size_t CHANGE_CURRENT_COLOR_MAP_GROUP_EVERY_N_UPDATES = 400U;

constexpr auto MIN_RADIUS_FACTOR       = 1.0F;
constexpr auto MAX_RADIUS_FACTOR       = 1.000001F;
constexpr auto MIN_TENTACLE_GROUP_SIZE = 10U;
constexpr auto TENTACLE_2D_X_MIN       = 0.0;
constexpr auto TENTACLE_2D_Y_MIN       = 0.065736;
constexpr auto TENTACLE_2D_Y_MAX       = 10000.0;
constexpr auto TENTACLE_LENGTH         = 120.0F;
constexpr auto NUM_TENTACLE_NODES      = 100U;
constexpr auto MAX_LINE_THICKNESS      = 5U;
constexpr auto PROB_THICK_LINES        = 0.9F;

constexpr auto MIN_SINE_FREQUENCY          = 1.0F;
constexpr auto MAX_SINE_FREQUENCY          = 3.1F;
constexpr auto MIN_BASE_Y_WEIGHT_FACTOR    = 0.8F;
constexpr auto MAX_BASE_Y_WEIGHT_FACTOR    = 1.1F;
constexpr auto ITER_ZERO_LERP_FACTOR       = 0.9;
constexpr auto MIN_SINE_X0                 = 0.0F;
const auto ITER_ZERO_Y_VAL_WAVE_ZERO_START = SineWaveMultiplier{
    SineWaveMultiplier::SineProperties{MIN_SINE_FREQUENCY, -20.0F, +20.0F, MIN_SINE_X0}
};

constexpr auto GetMatchingBaseYWeights(const float freq) noexcept -> Tentacle2D::BaseYWeights
{
  constexpr auto FREQUENCIES = std::array{
      1.0F,
      1.7F,
      2.3F,
      3.1F,
  };
  static_assert(FREQUENCIES.front() == MIN_SINE_FREQUENCY);
  static_assert(FREQUENCIES.back() == MAX_SINE_FREQUENCY);
  constexpr auto CORRESPONDING_BASE_Y_WEIGHTS = std::array{
      0.60F,
      0.70F,
      0.75F,
      0.80F,
  };
  constexpr auto HIGHEST_BASE_Y_WEIGHT = 0.85F;

  for (auto i = 0U; i < FREQUENCIES.size(); ++i)
  {
    if (freq <= FREQUENCIES.at(i))
    {
      return {CORRESPONDING_BASE_Y_WEIGHTS.at(i), 1.0F - CORRESPONDING_BASE_Y_WEIGHTS.at(i)};
    }
  }

  return {HIGHEST_BASE_Y_WEIGHT, 1.0F - HIGHEST_BASE_Y_WEIGHT};
}

} // namespace

TentacleDriver::TentacleDriver(IGoomDraw& draw,
                               const IGoomRand& goomRand,
                               const CirclesTentacleLayout& tentacleLayout) noexcept
  : m_goomRand{&goomRand},
    m_screenCentre{draw.GetDimensions().GetCentrePoint()},
    m_tentacleParams{
        NUM_TENTACLE_NODES, TENTACLE_LENGTH, MIN_SINE_FREQUENCY, ITER_ZERO_Y_VAL_WAVE_ZERO_START},
    m_tentaclePlotter{draw, *m_goomRand},
    m_tentacles{GetTentacles(*m_goomRand, tentacleLayout, m_tentacleParams)}
{
}

auto TentacleDriver::GetTentacles(const IGoomRand& goomRand,
                                  const CirclesTentacleLayout& tentacleLayout,
                                  const IterationParams& tentacleParams) noexcept
    -> std::vector<TentacleAndAttributes>
{
  const auto numTentacles = tentacleLayout.GetNumTentacles();

  auto tentacles = std::vector<TentacleAndAttributes>{};
  tentacles.reserve(numTentacles);

  for (auto i = 0U; i < numTentacles; ++i)
  {
    auto tentacle2D = CreateNewTentacle2D(goomRand, tentacleParams);
    tentacle2D->SetIterZeroLerpFactor(ITER_ZERO_LERP_FACTOR);

    auto tentacle = Tentacle3D{std::move(tentacle2D)};

    tentacle.SetStartPos(tentacleLayout.GetStartPoints().at(i));
    tentacle.SetEndPos(tentacleLayout.GetEndPoints().at(i));

    tentacles.emplace_back(TentacleAndAttributes{std::move(tentacle),
                                                 ColorMapSharedPtrWrapper{nullptr},
                                                 ColorMapSharedPtrWrapper{nullptr},
                                                 BLACK_PIXEL,
                                                 BLACK_PIXEL});
  }

  return tentacles;
}

auto TentacleDriver::CreateNewTentacle2D(const IGoomRand& goomRand,
                                         const IterationParams& tentacleParams) noexcept
    -> std::unique_ptr<Tentacle2D>
{
  const auto tentacleLen = tentacleParams.length;
  Ensures(tentacleLen >= 1.0F);
  const auto tent2dXMax = TENTACLE_2D_X_MIN + static_cast<double>(tentacleLen);
  Ensures(tent2dXMax >= 1.0);

  const auto dimensions = Tentacle2D::Dimensions{
      {TENTACLE_2D_X_MIN,        tent2dXMax},
      {TENTACLE_2D_Y_MIN, TENTACLE_2D_Y_MAX},
  };

  auto baseYWeights = GetMatchingBaseYWeights(tentacleParams.iterZeroYValWaveFreq);
  baseYWeights.previous *=
      goomRand.GetRandInRange(MIN_BASE_Y_WEIGHT_FACTOR, MAX_BASE_Y_WEIGHT_FACTOR);
  baseYWeights.current = 1.0F - baseYWeights.previous;

  return std::make_unique<Tentacle2D>(tentacleParams.numNodes, dimensions, baseYWeights);
}

auto TentacleDriver::SetWeightedColorMaps(
    const IVisualFx::WeightedColorMaps& weightedColorMaps) noexcept -> void
{
  const auto baseMainColorMapName = weightedColorMaps.mainColorMaps->GetRandomColorMapName();
  const auto baseLowColorMapName  = weightedColorMaps.lowColorMaps->GetRandomColorMapName();

  static constexpr auto MIN_SATURATION       = 0.2F;
  static constexpr auto SATURATION_STEP_SIZE = 0.1F;
  auto saturation                            = IncrementedValue<float>{
      MIN_SATURATION, 1.0F, TValue::StepType::CONTINUOUS_REVERSIBLE, SATURATION_STEP_SIZE};
  static constexpr auto MIN_LIGHTNESS       = 0.2F;
  static constexpr auto LIGHTNESS_STEP_SIZE = 0.1F;
  auto lightness                            = IncrementedValue<float>{
      MIN_LIGHTNESS, 1.0F, TValue::StepType::CONTINUOUS_REVERSIBLE, LIGHTNESS_STEP_SIZE};

  std::for_each(
      begin(m_tentacles),
      end(m_tentacles),
      [this, &baseMainColorMapName, &baseLowColorMapName, &saturation, &lightness](auto& tentacle)
      {
        const auto tintProperties = ColorMaps::TintProperties{saturation(), lightness()};

        tentacle.mainColorMap =
            m_colorMaps.GetTintedColorMapPtr(baseMainColorMapName, tintProperties);
        tentacle.lowColorMap =
            m_colorMaps.GetTintedColorMapPtr(baseLowColorMapName, tintProperties);

        saturation.Increment();
        lightness.Increment();
      });

  m_currentColorT.Reset();
}

auto TentacleDriver::StartIterating() -> void
{
  std::for_each(begin(m_tentacles),
                end(m_tentacles),
                [](auto& tentacle) { tentacle.tentacle3D.StartIterating(); });

  m_updateNum = 0;
  m_endCentrePosT.Reset();
}

auto TentacleDriver::MultiplyIterZeroYValWaveFreq(const float value) -> void
{
  const auto newFreq = std::clamp(
      value * m_tentacleParams.iterZeroYValWaveFreq, MIN_SINE_FREQUENCY, MAX_SINE_FREQUENCY);
  m_tentacleParams.iterZeroYValWave.SetFrequency(newFreq);

  for (auto& tentacle : m_tentacles)
  {
    tentacle.tentacle3D.SetBaseYWeights(GetMatchingBaseYWeights(newFreq));
  }
}

auto TentacleDriver::CheckForTimerEvents() -> void
{
  if ((m_updateNum % CHANGE_CURRENT_COLOR_MAP_GROUP_EVERY_N_UPDATES) != 0U)
  {
    return;
  }

  ChangeTentacleColorMaps();

  m_tentaclePlotter.UpdateCameraPosition();
}

auto TentacleDriver::ChangeTentacleColorMaps() -> void
{
  ChangeSegmentMixes();

  m_tentacleGroupSize = m_goomRand->GetRandInRange(MIN_TENTACLE_GROUP_SIZE,
                                                   static_cast<uint32_t>(m_tentacles.size()));

  m_useThickLines = m_goomRand->ProbabilityOf(PROB_THICK_LINES);
}

auto TentacleDriver::SetTentaclesEndCentrePos(const Point2dInt& newEndCentrePos) noexcept -> void
{
  m_targetEndCentrePos   = GetAcceptableEndCentrePos(newEndCentrePos);
  m_previousEndCentrePos = lerp(m_previousEndCentrePos, m_targetEndCentrePos, m_endCentrePosT());
  m_endCentrePosT.Reset();
}

inline auto TentacleDriver::GetAcceptableEndCentrePos(
    const Point2dInt& requestedEndCentrePos) const noexcept -> Point2dInt
{
  static constexpr auto CLOSE_TO_SCREEN_CENTRE_T = 0.2F;
  return lerp(requestedEndCentrePos, m_screenCentre, CLOSE_TO_SCREEN_CENTRE_T);
}

auto TentacleDriver::UpdateTentaclesEndCentrePosOffsets() noexcept -> void
{
  const auto endCentrePos = lerp(m_previousEndCentrePos, m_targetEndCentrePos, m_endCentrePosT());
  const auto endCentrePosOffset = endCentrePos - ToVec2dInt(m_screenCentre);
  const auto radiusScale        = m_goomRand->GetRandInRange(MIN_RADIUS_FACTOR, MAX_RADIUS_FACTOR);

  std::for_each(begin(m_tentacles),
                end(m_tentacles),
                [&endCentrePosOffset, &radiusScale](auto& tentacle)
                {
                  const auto newRadiusCentreEndPosOffset = GetNewRadiusEndCentrePosOffset(
                      radiusScale, tentacle.tentacle3D.GetEndPos(), endCentrePosOffset);

                  tentacle.tentacle3D.SetEndPosOffset(newRadiusCentreEndPosOffset);
                });
}

auto TentacleDriver::UpdateTentaclesEndPos() noexcept -> void
{
  std::for_each(begin(m_tentacles),
                end(m_tentacles),
                [this](auto& tentacle)
                {
                  const auto tentacleEndPos = tentacle.tentacle3D.GetEndPos();
                  const auto endRadius      = std::sqrt(UTILS::MATH::Sq(tentacleEndPos.x) +
                                                   UTILS::MATH::Sq(tentacleEndPos.y));
                  const auto newTentacleEndPos =
                      (m_endRadius() / endRadius) * tentacle.tentacle3D.GetEndPos();

                  tentacle.tentacle3D.SetEndPos(newTentacleEndPos);
                });
}

auto TentacleDriver::GetNewRadiusEndCentrePosOffset(
    const float radiusScale,
    const Point2dFlt& oldTentacleEndPos,
    const Point2dInt& newCentreEndPosOffset) noexcept -> V3dFlt
{
  const auto oldTentacleEndPosVec = Vec2dInt{static_cast<int32_t>(oldTentacleEndPos.x),
                                             static_cast<int32_t>(oldTentacleEndPos.y)};
  const auto newTentacleEndPos    = Point2dInt{
      static_cast<int32_t>(radiusScale * static_cast<float>(oldTentacleEndPosVec.x)),
      static_cast<int32_t>(radiusScale * static_cast<float>(oldTentacleEndPosVec.y)),
  };
  const auto newRadiusEndPosOffset = newTentacleEndPos - oldTentacleEndPosVec;

  const auto newRadiusCentreEndPosOffset =
      newCentreEndPosOffset + ToVec2dInt(newRadiusEndPosOffset);

  return V3dFlt{static_cast<float>(newRadiusCentreEndPosOffset.x),
                static_cast<float>(newRadiusCentreEndPosOffset.y),
                0.0F};
}

auto TentacleDriver::Update() -> void
{
  ++m_updateNum;

  CheckForTimerEvents();

  PreDrawUpdateTentacles();
  DrawTentacles();
  PostDrawUpdateTentacles();
}

inline auto TentacleDriver::PreDrawUpdateTentacles() noexcept -> void
{
  UpdateTentaclesEndCentrePosOffsets();
  UpdateTentaclesEndPos();
}

auto TentacleDriver::DrawTentacles() noexcept -> void
{
  auto colorT = TValue{
      {TValue::StepType::CONTINUOUS_REVERSIBLE, m_tentacleGroupSize}
  };

  m_tentaclePlotter.SetNodeTOffset(m_nodeTOffset());

  for (auto i = 0U; i < m_tentacles.size(); ++i)
  {
    auto& tentacleAndAttributes = m_tentacles.at(i);

    tentacleAndAttributes.currentMainColor =
        tentacleAndAttributes.mainColorMap.GetColor(m_currentColorT());
    tentacleAndAttributes.currentLowColor =
        tentacleAndAttributes.lowColorMap.GetColor(m_currentColorT());

    IterateTentacle(tentacleAndAttributes.tentacle3D);

    m_tentaclePlotter.SetEndDotColors(
        {m_dominantMainColorMap.GetColor(colorT()), m_dominantLowColorMap.GetColor(colorT())});

    m_tentaclePlotter.SetTentacleLineThickness(GetLineThickness(i));

    static constexpr auto BRIGHTNESS = 5.0F;
    m_tentaclePlotter.SetGetColorsFunc(
        [this, &colorT, &tentacleAndAttributes](const float nodeT)
        { return GetMixedColors(colorT(), nodeT, tentacleAndAttributes, BRIGHTNESS); });

    m_tentaclePlotter.Plot3D(tentacleAndAttributes.tentacle3D);

    colorT.Increment();
  }
}

inline auto TentacleDriver::PostDrawUpdateTentacles() noexcept -> void
{
  m_tentacleParams.iterZeroYValWave.Increment();
  m_endCentrePosT.Increment();
  m_endRadius.Increment();
  m_currentColorT.Increment();
  m_nodeTOffset.Increment();
}

inline auto TentacleDriver::IterateTentacle(Tentacle3D& tentacle) const noexcept -> void
{
  tentacle.SetIterZeroYVal(m_tentacleParams.iterZeroYValWave.GetNext());
  tentacle.Iterate();
}

auto TentacleDriver::GetMixedColors(const float dominantT,
                                    const float nodeT,
                                    const TentacleAndAttributes& tentacleAndAttributes,
                                    const float brightness) const -> MultiplePixels
{
  auto mixedColors =
      MultiplePixels{IColorMap::GetColorMix(m_dominantMainColorMap.GetColor(dominantT),
                                            tentacleAndAttributes.mainColorMap.GetColor(nodeT),
                                            m_mainColorSegmentMixT),
                     IColorMap::GetColorMix(m_dominantLowColorMap.GetColor(dominantT),
                                            tentacleAndAttributes.lowColorMap.GetColor(nodeT),
                                            m_lowColorSegmentMixT)};

  mixedColors.color1.SetA(MAX_ALPHA / 10);
  mixedColors.color2.SetA(MAX_ALPHA / 10);

  return {
      m_colorAdjust.GetAdjustment(MAIN_BRIGHTNESS_FACTOR * brightness, GetMainColor(mixedColors)),
      m_colorAdjust.GetAdjustment(LOW_BRIGHTNESS_FACTOR * brightness, GetLowColor(mixedColors))};
}

inline auto TentacleDriver::ChangeSegmentMixes() noexcept -> void
{
  m_mainColorSegmentMixT =
      m_goomRand->GetRandInRange(MIN_COLOR_SEGMENT_MIX_T, MAX_COLOR_SEGMENT_MIX_T);

  m_lowColorSegmentMixT =
      m_goomRand->ProbabilityOf(PROB_LOW_MIX_SAME)
          ? m_mainColorSegmentMixT
          : m_goomRand->GetRandInRange(MIN_COLOR_SEGMENT_MIX_T, MAX_COLOR_SEGMENT_MIX_T);
}

inline auto TentacleDriver::GetLineThickness(const uint32_t tentacleNum) const noexcept -> uint8_t
{
  if (not m_useThickLines)
  {
    return 1U;
  }

  static constexpr auto TWICE_MAX_THICKNESS = 2U * MAX_LINE_THICKNESS;

  auto lineThickness = static_cast<uint8_t>(1U + (tentacleNum % TWICE_MAX_THICKNESS));
  if (lineThickness <= MAX_LINE_THICKNESS)
  {
    return lineThickness;
  }

  lineThickness = TWICE_MAX_THICKNESS - lineThickness;
  if (0 == lineThickness)
  {
    lineThickness = 1U;
  }
  return lineThickness;
}

} // namespace GOOM::VISUAL_FX::TENTACLES
