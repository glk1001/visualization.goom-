module;

#include <cmath>
#include <cstdint>
#include <format>
#include <string>

module Goom.FilterFx.AfterEffects.TheEffects.Hypercos;

import Goom.FilterFx.AfterEffects.AfterEffectsStates;
import Goom.FilterFx.CommonTypes;
import Goom.FilterFx.NormalizedCoords;
import Goom.Utils.EnumUtils;
import Goom.Utils.NameValuePairs;
import Goom.Utils.Math.GoomRand;
import Goom.Utils.Math.Misc;
import Goom.Lib.Point2d;

namespace GOOM::FILTER_FX::AFTER_EFFECTS
{

using UTILS::GetFullParamGroup;
using UTILS::GetPair;
using UTILS::NameValuePairs;
using UTILS::NUM;
using UTILS::MATH::GoomRand;
using UTILS::MATH::NumberRange;
using UTILS::MATH::PI;

using enum Hypercos::HypercosEffect;

// Hypercos:
// applique une surcouche de hypercos effect
// applies an overlay of hypercos effect
static constexpr auto X_DEFAULT_AMPLITUDE = 1.0F / 120.0F;
static constexpr auto Y_DEFAULT_AMPLITUDE = 1.0F / 120.0F;
static constexpr auto AMPLITUDE_RANGE =
    NumberRange{0.1F * X_DEFAULT_AMPLITUDE, 1.1F * X_DEFAULT_AMPLITUDE};
static constexpr auto BIG_AMPLITUDE_RANGE =
    NumberRange{0.1F * X_DEFAULT_AMPLITUDE, 10.1F * X_DEFAULT_AMPLITUDE};

static constexpr auto X_DEFAULT_FREQUENCY_FACTOR      = 10.0F;
static constexpr auto Y_DEFAULT_FREQUENCY_FACTOR      = 10.0F;
static constexpr auto FREQUENCY_FACTOR_RANGE          = NumberRange{5.0F, 100.0F};
static constexpr auto BIG_FREQUENCY_FACTOR_RANGE      = NumberRange{5.0F, 500.0F};
static constexpr auto VERY_BIG_FREQUENCY_FACTOR_RANGE = NumberRange{1000.0F, 10000.0F};

static constexpr auto DEFAULT_OVERLAY = HypercosOverlayMode::NONE;
static constexpr auto DEFAULT_EFFECT  = NONE;
static constexpr auto DEFAULT_REVERSE = false;

static constexpr auto PROB_FREQUENCY_FACTORS_EQUAL = 0.5F;
static constexpr auto PROB_REVERSE                 = 0.5F;
static constexpr auto PROB_AMPLITUDES_EQUAL        = 0.5F;
static constexpr auto PROB_BIG_AMPLITUDE_RANGE     = 0.2F;

static constexpr auto DEFAULT_PARAMS = Hypercos::Params{
    .amplitude       = {       .x = X_DEFAULT_AMPLITUDE,        .y = Y_DEFAULT_AMPLITUDE},
    .frequencyFactor = {.x = X_DEFAULT_FREQUENCY_FACTOR, .y = Y_DEFAULT_FREQUENCY_FACTOR},
    .overlay         = DEFAULT_OVERLAY,
    .effect          = DEFAULT_EFFECT,
    .reverse         = DEFAULT_REVERSE,
};

static constexpr auto HYPERCOS_EFFECT_NONE_WEIGHT               = 00.0F;
static constexpr auto HYPERCOS_EFFECT_SIN_CURL_SWIRL_WEIGHT     = 15.0F;
static constexpr auto HYPERCOS_EFFECT_COS_CURL_SWIRL_WEIGHT     = 15.0F;
static constexpr auto HYPERCOS_EFFECT_SIN_COS_CURL_SWIRL_WEIGHT = 15.0F;
static constexpr auto HYPERCOS_EFFECT_COS_SIN_CURL_SWIRL_WEIGHT = 15.0F;
static constexpr auto HYPERCOS_EFFECT_SIN_TAN_CURL_SWIRL_WEIGHT = 05.0F;
static constexpr auto HYPERCOS_EFFECT_COS_TAN_CURL_SWIRL_WEIGHT = 05.0F;
static constexpr auto HYPERCOS_EFFECT_SIN_RECTANGULAR_WEIGHT    = 05.0F;
static constexpr auto HYPERCOS_EFFECT_COS_RECTANGULAR_WEIGHT    = 05.0F;
static constexpr auto HYPERCOS_EFFECT_SIN_OF_COS_SWIRL_WEIGHT   = 15.0F;
static constexpr auto HYPERCOS_EFFECT_COS_OF_SIN_SWIRL_WEIGHT   = 15.0F;

Hypercos::Hypercos(const GoomRand& goomRand) noexcept
  : m_goomRand{&goomRand},
    m_params{DEFAULT_PARAMS},
    m_hypercosOverlayWeights{
        *m_goomRand,
        {
            { .key = NONE,               .weight = HYPERCOS_EFFECT_NONE_WEIGHT },
            { .key = SIN_CURL_SWIRL,     .weight = HYPERCOS_EFFECT_SIN_CURL_SWIRL_WEIGHT },
            { .key = COS_CURL_SWIRL,     .weight = HYPERCOS_EFFECT_COS_CURL_SWIRL_WEIGHT },
            { .key = SIN_COS_CURL_SWIRL, .weight = HYPERCOS_EFFECT_SIN_COS_CURL_SWIRL_WEIGHT },
            { .key = COS_SIN_CURL_SWIRL, .weight = HYPERCOS_EFFECT_COS_SIN_CURL_SWIRL_WEIGHT },
            { .key = SIN_TAN_CURL_SWIRL, .weight = HYPERCOS_EFFECT_SIN_TAN_CURL_SWIRL_WEIGHT },
            { .key = COS_TAN_CURL_SWIRL, .weight = HYPERCOS_EFFECT_COS_TAN_CURL_SWIRL_WEIGHT },
            { .key = SIN_RECTANGULAR,    .weight = HYPERCOS_EFFECT_SIN_RECTANGULAR_WEIGHT },
            { .key = COS_RECTANGULAR,    .weight = HYPERCOS_EFFECT_COS_RECTANGULAR_WEIGHT },
            { .key = SIN_OF_COS_SWIRL,   .weight = HYPERCOS_EFFECT_SIN_OF_COS_SWIRL_WEIGHT },
            { .key = COS_OF_SIN_SWIRL,   .weight = HYPERCOS_EFFECT_COS_OF_SIN_SWIRL_WEIGHT },
        }
    }
{
}

auto Hypercos::SetDefaultParams() -> void
{
  SetParams(DEFAULT_PARAMS);
}

auto Hypercos::GetMode0RandomParams() const noexcept -> Params
{
  const auto hypercosMax = std::lerp(FREQUENCY_FACTOR_RANGE.min, FREQUENCY_FACTOR_RANGE.max, 0.15F);

  return GetHypercosEffect(
      HypercosOverlayMode::MODE0, {FREQUENCY_FACTOR_RANGE.min, hypercosMax}, AMPLITUDE_RANGE);
}

auto Hypercos::GetMode1RandomParams() const noexcept -> Params
{
  const auto hypercosMin = std::lerp(FREQUENCY_FACTOR_RANGE.min, FREQUENCY_FACTOR_RANGE.max, 0.20F);

  return GetHypercosEffect(
      HypercosOverlayMode::MODE1, {hypercosMin, FREQUENCY_FACTOR_RANGE.max}, AMPLITUDE_RANGE);
}

auto Hypercos::GetMode2RandomParams() const noexcept -> Params
{
  const auto amplitudeRange =
      m_goomRand->ProbabilityOf<PROB_BIG_AMPLITUDE_RANGE>() ? BIG_AMPLITUDE_RANGE : AMPLITUDE_RANGE;

  const auto hypercosMin = std::lerp(FREQUENCY_FACTOR_RANGE.min, FREQUENCY_FACTOR_RANGE.max, 0.50F);

  return GetHypercosEffect(
      HypercosOverlayMode::MODE2, {hypercosMin, BIG_FREQUENCY_FACTOR_RANGE.max}, amplitudeRange);
}

auto Hypercos::GetMode3RandomParams() const noexcept -> Params
{
  return GetHypercosEffect(
      HypercosOverlayMode::MODE3, VERY_BIG_FREQUENCY_FACTOR_RANGE, AMPLITUDE_RANGE);
}

auto Hypercos::GetHypercosEffect(HypercosOverlayMode overlay,
                                 const NumberRange<float>& freqRange,
                                 const NumberRange<float>& amplitudeRange) const noexcept -> Params
{
  const auto xAmplitude = m_goomRand->GetRandInRange(amplitudeRange);
  const auto yAmplitude = m_goomRand->ProbabilityOf<PROB_AMPLITUDES_EQUAL>()
                              ? xAmplitude
                              : m_goomRand->GetRandInRange(amplitudeRange);

  const auto xFrequencyFactor = m_goomRand->GetRandInRange(freqRange);
  const auto yFrequencyFactor = m_goomRand->ProbabilityOf<PROB_FREQUENCY_FACTORS_EQUAL>()
                                    ? xFrequencyFactor
                                    : m_goomRand->GetRandInRange(freqRange);

  const auto reverse = m_goomRand->ProbabilityOf<PROB_REVERSE>();

  return {
      .amplitude       = {      .x = xAmplitude,       .y = yAmplitude},
      .frequencyFactor = {.x = xFrequencyFactor, .y = yFrequencyFactor},
      .overlay         = overlay,
      .effect          = m_hypercosOverlayWeights.GetRandomWeighted(),
      .reverse         = reverse,
  };
}

inline auto Hypercos::GetFrequencyFactorToUse(const float frequencyFactor) const -> float
{
  return m_params.reverse ? -frequencyFactor : +frequencyFactor;
}

auto Hypercos::GetVelocity(const NormalizedCoords& coords, const NormalizedCoords& velocity) const
    -> NormalizedCoords
{
  const auto xFrequencyFactorToUse = GetFrequencyFactorToUse(m_params.frequencyFactor.x);
  const auto yFrequencyFactorToUse = GetFrequencyFactorToUse(m_params.frequencyFactor.y);

  return velocity + GetVelocity(coords,
                                m_params.effect,
                                {.x = xFrequencyFactorToUse, .y = yFrequencyFactorToUse});
}

auto Hypercos::GetVelocity(const NormalizedCoords& coords,
                           const HypercosEffect effect,
                           const FrequencyFactor& frequencyFactorToUse) const -> NormalizedCoords
{
  auto xVal = 0.0F;
  auto yVal = 0.0F;

  switch (effect)
  {
    case NONE:
      break;
    case SIN_RECTANGULAR:
      xVal = std::sin(frequencyFactorToUse.x * coords.GetX());
      yVal = std::sin(frequencyFactorToUse.y * coords.GetY());
      break;
    case COS_RECTANGULAR:
      xVal = std::cos(frequencyFactorToUse.x * coords.GetX());
      yVal = std::cos(frequencyFactorToUse.y * coords.GetY());
      break;
    case SIN_CURL_SWIRL:
      xVal = std::sin(frequencyFactorToUse.y * coords.GetY());
      yVal = std::sin(frequencyFactorToUse.x * coords.GetX());
      break;
    case COS_CURL_SWIRL:
      xVal = std::cos(frequencyFactorToUse.y * coords.GetY());
      yVal = std::cos(frequencyFactorToUse.x * coords.GetX());
      break;
    case SIN_COS_CURL_SWIRL:
      xVal = std::sin(frequencyFactorToUse.x * coords.GetY());
      yVal = std::cos(frequencyFactorToUse.y * coords.GetX());
      break;
    case COS_SIN_CURL_SWIRL:
      xVal = std::cos(frequencyFactorToUse.y * coords.GetY());
      yVal = std::sin(frequencyFactorToUse.x * coords.GetX());
      break;
    case SIN_TAN_CURL_SWIRL:
      xVal = std::sin(std::tan(frequencyFactorToUse.y * coords.GetY()));
      yVal = std::cos(std::tan(frequencyFactorToUse.x * coords.GetX()));
      break;
    case COS_TAN_CURL_SWIRL:
      xVal = std::cos(std::tan(frequencyFactorToUse.y * coords.GetY()));
      yVal = std::sin(std::tan(frequencyFactorToUse.x * coords.GetX()));
      break;
    case SIN_OF_COS_SWIRL:
      xVal = std::sin(PI * std::cos(frequencyFactorToUse.y * coords.GetY()));
      yVal = std::cos(PI * std::sin(frequencyFactorToUse.x * coords.GetX()));
      break;
    case COS_OF_SIN_SWIRL:
      xVal = std::cos(PI * std::sin(frequencyFactorToUse.y * coords.GetY()));
      yVal = std::sin(PI * std::cos(frequencyFactorToUse.x * coords.GetX()));
      break;
  }

  //  xVal = std::clamp(std::tan(hypercosFreqY * xVal), -1.0, 1.0);
  //  yVal = std::clamp(std::tan(hypercosFreqX * yVal), -1.0, 1.0);

  xVal *= m_params.amplitude.x;
  yVal *= m_params.amplitude.y;

  return {xVal, yVal};
}

auto Hypercos::GetNameValueParams(const std::string& paramGroup) const -> NameValuePairs
{
  const auto fullParamGroup = GetFullParamGroup({paramGroup, "hypercos"});

  if (m_params.overlay == HypercosOverlayMode::NONE)
  {
    return {GetPair(fullParamGroup, "overlay", std::string{"None"})};
  }

  return {GetPair(fullParamGroup,
                  "Params",
                  std::format("({:.3f},{:.3f}), ({:.1f},{:.1f}), {}, {}, {}",
                              m_params.amplitude.x,
                              m_params.amplitude.y,
                              m_params.frequencyFactor.x,
                              m_params.frequencyFactor.y,
                              static_cast<uint32_t>(m_params.overlay),
                              static_cast<uint32_t>(m_params.effect),
                              m_params.reverse))};
}

} // namespace GOOM::FILTER_FX::AFTER_EFFECTS
