module;

#include <cmath>
#include <cstdint>
#include <utility>

module Goom.FilterFx.FilterEffects.AdjustmentEffects.YOnly;

import Goom.FilterFx.CommonTypes;
import Goom.FilterFx.NormalizedCoords;
import Goom.Utils.EnumUtils;
import Goom.Utils.NameValuePairs;
import Goom.Utils.Math.GoomRand;

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

using UTILS::NameValuePairs;
using UTILS::NUM;
using UTILS::MATH::GoomRand;
using UTILS::MATH::NumberRange;

static constexpr auto FREQ_FACTOR_RANGE = FrequencyFactorRange{
    .xRange = {-50.0F, +50.01F},
    .yRange = {-50.0F, +50.01F},
};

static constexpr auto AMPLITUDE_RANGE = AmplitudeRange{
    .xRange = {0.010F, 1.011F},
    .yRange = {0.010F, 1.011F},
};

static constexpr auto PROB_Y_ONLY_STRICT   = 0.9F;
static constexpr auto PROB_FREQ_EQUAL      = 0.9F;
static constexpr auto PROB_AMPLITUDE_EQUAL = 0.9F;

YOnly::YOnly(const GoomRand& goomRand) noexcept : m_goomRand{&goomRand}, m_params{GetRandomParams()}
{
}

auto YOnly::GetRandomParams() const noexcept -> Params
{
  static constexpr auto Y_ONLY_EFFECT_RANGE =
      NumberRange{static_cast<uint32_t>(YOnlyEffect::NONE) + 1, NUM<YOnlyEffect> - 1};
  const auto xEffect = static_cast<YOnlyEffect>(m_goomRand->GetRandInRange<Y_ONLY_EFFECT_RANGE>());
  const auto yEffect =
      m_goomRand->ProbabilityOf<PROB_Y_ONLY_STRICT>()
          ? YOnlyEffect::NONE
          : static_cast<YOnlyEffect>(m_goomRand->GetRandInRange<Y_ONLY_EFFECT_RANGE>());

  const auto xFreqFactor = m_goomRand->GetRandInRange<FREQ_FACTOR_RANGE.xRange>();
  const auto yFreqFactor = m_goomRand->ProbabilityOf<PROB_FREQ_EQUAL>()
                               ? xFreqFactor
                               : m_goomRand->GetRandInRange<FREQ_FACTOR_RANGE.yRange>();

  const auto xAmplitude = m_goomRand->GetRandInRange<AMPLITUDE_RANGE.xRange>();
  const auto yAmplitude = m_goomRand->ProbabilityOf<PROB_AMPLITUDE_EQUAL>()
                              ? xAmplitude
                              : m_goomRand->GetRandInRange<AMPLITUDE_RANGE.yRange>();

  return {
      .xyEffect        = {.xEffect = xEffect, .yEffect = yEffect},
      .frequencyFactor = {  .x = xFreqFactor,   .y = yFreqFactor},
      .amplitude       = {        xAmplitude,         yAmplitude}
  };
}

auto YOnly::GetYOnlyZoomAdjustmentMultiplier(const YOnlyEffect effect,
                                             const NormalizedCoords& coords) const noexcept -> float
{
  switch (effect)
  {
    case YOnlyEffect::NONE:
      std::unreachable();
    case YOnlyEffect::X_SIN_Y_SIN:
      return std::sin(m_params.frequencyFactor.x * coords.GetX()) *
             std::sin(m_params.frequencyFactor.y * coords.GetY());
    case YOnlyEffect::X_SIN_Y_COS:
      return std::sin(m_params.frequencyFactor.x * coords.GetX()) *
             std::cos(m_params.frequencyFactor.y * coords.GetY());
    case YOnlyEffect::X_COS_Y_SIN:
      return std::cos(m_params.frequencyFactor.x * coords.GetX()) *
             std::sin(m_params.frequencyFactor.y * coords.GetY());
    case YOnlyEffect::X_COS_Y_COS:
      return std::cos(m_params.frequencyFactor.x * coords.GetX()) *
             std::cos(m_params.frequencyFactor.y * coords.GetY());
  }
}

auto YOnly::GetZoomAdjustmentEffectNameValueParams() const noexcept -> NameValuePairs
{
  return {};
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
