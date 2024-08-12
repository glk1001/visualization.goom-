module;

#include <algorithm>
#include <cmath>
#include <mdspan.hpp>

module Goom.FilterFx.FilterEffects.AdjustmentEffects.TestFlowField;

import Goom.FilterFx.FilterUtils.Utils;
import Goom.FilterFx.NormalizedCoords;
import Goom.Utils.NameValuePairs;
import Goom.Utils.Math.GoomRand;
import Goom.Utils.Math.Misc;
import Goom.Lib.AssertUtils;
import Goom.Lib.Point2d;

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{
using UTILS::NameValuePairs;
using UTILS::MATH::GetRandSeed;
using UTILS::MATH::GoomRand;
using UTILS::MATH::NumberRange;
using UTILS::MATH::Sq;
using UTILS::MATH::TWO_PI;

namespace
{

constexpr auto DEFAULT_AMPLITUDE = Amplitude{0.1F, 0.1F};
constexpr auto AMPLITUDE_RANGE   = NumberRange{0.05F, 0.251F};

constexpr auto DEFAULT_LERP_TO_ONE_T_S = LerpToOneTs{.xLerpT = 0.5F, .yLerpT = 0.5F};
constexpr auto LERP_TO_ONE_T_RANGE     = NumberRange{0.0F, 1.0F};

constexpr auto DEFAULT_ANGLE_FREQUENCY_FACTOR = 5.0F;
constexpr auto ANGLE_FREQUENCY_FACTOR_RANGE   = NumberRange{0.1F, 20.0F};

constexpr auto PROB_XY_AMPLITUDES_EQUAL        = 0.98F;
constexpr auto PROB_LERP_TO_ONE_T_S_EQUAL      = 0.95F;
constexpr auto PROB_XY_ANGLE_FREQUENCIES_EQUAL = 0.5F;

} // namespace

TestFlowField::TestFlowField(const GoomRand& goomRand) noexcept
: m_goomRand{&goomRand},
  m_params{.amplitude = DEFAULT_AMPLITUDE,
           .lerpToOneTs = DEFAULT_LERP_TO_ONE_T_S,
           .angleFrequencyFactor = {.x = DEFAULT_ANGLE_FREQUENCY_FACTOR,
                                      .y = DEFAULT_ANGLE_FREQUENCY_FACTOR}}
{
  SetupAngles();
}

auto TestFlowField::SetupAngles() noexcept -> void
{
  auto gridAngles = std::mdspan{m_gridArray.data(), GRID_HEIGHT, GRID_WIDTH};

  Expects(gridAngles.extent(0) == GRID_HEIGHT);
  Expects(gridAngles.extent(1) == GRID_WIDTH);

  for (auto col = 0U; col < GRID_WIDTH; ++col)
  {
    const auto x = 2.0F * (-0.5F + (static_cast<float>(col) / static_cast<float>(GRID_WIDTH)));

    for (auto row = 0U; row < GRID_HEIGHT; ++row)
    {
      const auto y = 2.0F * (-0.5F + (static_cast<float>(row) / static_cast<float>(GRID_HEIGHT)));

      const auto distFromCentre = std::sqrt(Sq(x) + Sq(y));
      const auto angle = std::sin(distFromCentre * TWO_PI) + std::cos(distFromCentre * TWO_PI);

      gridAngles[row, col] = angle;
    }
  }
}

auto TestFlowField::GetVelocity(const Vec2dFlt& baseZoomAdjustment,
                                const NormalizedCoords& coords) const noexcept -> Vec2dFlt
{
  const auto gridCoords =
      ToPoint2dInt(m_normalizedCoordsToGridConverter.NormalizedToOtherCoordsFlt(coords));

  const auto gridAngles = std::mdspan{m_gridArray.data(), GRID_HEIGHT, GRID_WIDTH};
  const auto gridAngle  = gridAngles[std::clamp(gridCoords.y, 0, static_cast<int>(GRID_HEIGHT) - 1),
                                    std::clamp(gridCoords.x, 0, static_cast<int>(GRID_WIDTH) - 1)];

  const auto sqDistFromZero = std::sqrt(SqDistanceFromZero(coords));

  return {.x = baseZoomAdjustment.x + (m_params.amplitude.x * sqDistFromZero *
                                       std::cos(m_params.angleFrequencyFactor.x * gridAngle)),
          .y = baseZoomAdjustment.y + (m_params.amplitude.y * sqDistFromZero *
                                       std::sin(m_params.angleFrequencyFactor.y * gridAngle))};

  // return {coords.GetX() * (m_params.amplitude.x * sqDistFromZero*std::cos(15.0F * gridAngle)),
  //         coords.GetY() * + (m_params.amplitude.y * sqDistFromZero*std::sin(25.0F * gridAngle))};
}

auto TestFlowField::SetRandomParams() noexcept -> void
{
  const auto xAmplitude = m_goomRand->GetRandInRange<AMPLITUDE_RANGE>();
  const auto yAmplitude = m_goomRand->ProbabilityOf<PROB_XY_AMPLITUDES_EQUAL>()
                              ? xAmplitude
                              : m_goomRand->GetRandInRange<AMPLITUDE_RANGE>();

  const auto xLerpToOneT = m_goomRand->GetRandInRange<LERP_TO_ONE_T_RANGE>();
  const auto yLerpToOneT = m_goomRand->ProbabilityOf<PROB_LERP_TO_ONE_T_S_EQUAL>()
                               ? xLerpToOneT
                               : m_goomRand->GetRandInRange<LERP_TO_ONE_T_RANGE>();

  const auto xAngleFrequencyFactor = m_goomRand->GetRandInRange<ANGLE_FREQUENCY_FACTOR_RANGE>();
  const auto yAngleFrequencyFactor =
      m_goomRand->ProbabilityOf<PROB_XY_ANGLE_FREQUENCIES_EQUAL>()
          ? xAngleFrequencyFactor
          : m_goomRand->GetRandInRange<ANGLE_FREQUENCY_FACTOR_RANGE>();

  SetParams({
      .amplitude            = {                xAmplitude,                 yAmplitude},
      .lerpToOneTs          = {     .xLerpT = xLerpToOneT,      .yLerpT = yLerpToOneT},
      .angleFrequencyFactor = {.x = xAngleFrequencyFactor, .y = yAngleFrequencyFactor},
  });
}

auto TestFlowField::GetZoomAdjustmentEffectNameValueParams() const noexcept -> NameValuePairs
{
  return {};
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
