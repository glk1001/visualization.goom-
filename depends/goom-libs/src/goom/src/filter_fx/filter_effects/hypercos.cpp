#include "hypercos.h"

#include "utils/enumutils.h"
#include "utils/math/misc.h"
#include "utils/name_value_pairs.h"

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

using STD20::pi;
using UTILS::GetFullParamGroup;
using UTILS::GetPair;
using UTILS::NameValuePairs;
using UTILS::NUM;
using UTILS::MATH::IGoomRand;

// Hypercos:
// applique une surcouche de hypercos effect
// applies an overlay of hypercos effect
static constexpr auto DEFAULT_OVERLAY = HypercosOverlay::NONE;
static constexpr auto DEFAULT_EFFECT  = Hypercos::HypercosEffect::NONE;
static constexpr auto DEFAULT_REVERSE = false;

static constexpr auto X_DEFAULT_FREQ      = 10.0F;
static constexpr auto Y_DEFAULT_FREQ      = 10.0F;
static constexpr auto FREQ_RANGE          = IGoomRand::NumberRange<float>{5.0F, 100.0F};
static constexpr auto BIG_FREQ_RANGE      = IGoomRand::NumberRange<float>{5.0F, 500.0F};
static constexpr auto VERY_BIG_FREQ_RANGE = IGoomRand::NumberRange<float>{30000.0F, 50000.0F};

static constexpr auto X_DEFAULT_AMPLITUDE = 1.0F / 120.0F;
static constexpr auto Y_DEFAULT_AMPLITUDE = 1.0F / 120.0F;
static constexpr auto AMPLITUDE_RANGE =
    IGoomRand::NumberRange<float>{0.1F * X_DEFAULT_AMPLITUDE, 1.1F * X_DEFAULT_AMPLITUDE};
static constexpr auto BIG_AMPLITUDE_RANGE =
    IGoomRand::NumberRange<float>{0.1F * X_DEFAULT_AMPLITUDE, 10.1F * X_DEFAULT_AMPLITUDE};

static constexpr auto PROB_FREQ_EQUAL          = 0.5F;
static constexpr auto PROB_REVERSE             = 0.5F;
static constexpr auto PROB_AMPLITUDE_EQUAL     = 0.5F;
static constexpr auto PROB_BIG_AMPLITUDE_RANGE = 0.2F;

// clang-format off
static constexpr auto DEFAULT_PARAMS = Hypercos::Params{
    DEFAULT_OVERLAY,
    DEFAULT_EFFECT,
    DEFAULT_REVERSE,
    X_DEFAULT_FREQ,
    Y_DEFAULT_FREQ,
    X_DEFAULT_AMPLITUDE,
    Y_DEFAULT_AMPLITUDE
};
// clang-format on

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

Hypercos::Hypercos(const IGoomRand& goomRand) noexcept
  : m_goomRand{goomRand},
    m_params{DEFAULT_PARAMS},
    m_hypercosOverlayWeights{
        m_goomRand,
        {
            { HypercosEffect::NONE,               HYPERCOS_EFFECT_NONE_WEIGHT },
            { HypercosEffect::SIN_CURL_SWIRL,     HYPERCOS_EFFECT_SIN_CURL_SWIRL_WEIGHT },
            { HypercosEffect::COS_CURL_SWIRL,     HYPERCOS_EFFECT_COS_CURL_SWIRL_WEIGHT },
            { HypercosEffect::SIN_COS_CURL_SWIRL, HYPERCOS_EFFECT_SIN_COS_CURL_SWIRL_WEIGHT },
            { HypercosEffect::COS_SIN_CURL_SWIRL, HYPERCOS_EFFECT_COS_SIN_CURL_SWIRL_WEIGHT },
            { HypercosEffect::SIN_TAN_CURL_SWIRL, HYPERCOS_EFFECT_SIN_TAN_CURL_SWIRL_WEIGHT },
            { HypercosEffect::COS_TAN_CURL_SWIRL, HYPERCOS_EFFECT_COS_TAN_CURL_SWIRL_WEIGHT },
            { HypercosEffect::SIN_RECTANGULAR,    HYPERCOS_EFFECT_SIN_RECTANGULAR_WEIGHT },
            { HypercosEffect::COS_RECTANGULAR,    HYPERCOS_EFFECT_COS_RECTANGULAR_WEIGHT },
            { HypercosEffect::SIN_OF_COS_SWIRL,   HYPERCOS_EFFECT_SIN_OF_COS_SWIRL_WEIGHT },
            { HypercosEffect::COS_OF_SIN_SWIRL,   HYPERCOS_EFFECT_COS_OF_SIN_SWIRL_WEIGHT },
        }
    }
{
}

auto Hypercos::SetDefaultParams() -> void
{
  SetParams(DEFAULT_PARAMS);
}

auto Hypercos::SetMode0RandomParams() -> void
{
  const auto hypercosMax = STD20::lerp(FREQ_RANGE.min, FREQ_RANGE.max, 0.15F);
  SetHypercosEffect(HypercosOverlay::MODE0, {FREQ_RANGE.min, hypercosMax}, AMPLITUDE_RANGE);
}

auto Hypercos::SetMode1RandomParams() -> void
{
  const auto hypercosMin = STD20::lerp(FREQ_RANGE.min, FREQ_RANGE.max, 0.20F);
  SetHypercosEffect(HypercosOverlay::MODE1, {hypercosMin, FREQ_RANGE.max}, AMPLITUDE_RANGE);
}

auto Hypercos::SetMode2RandomParams() -> void
{
  const auto amplitudeRange =
      m_goomRand.ProbabilityOf(PROB_BIG_AMPLITUDE_RANGE) ? BIG_AMPLITUDE_RANGE : AMPLITUDE_RANGE;

  const auto hypercosMin = STD20::lerp(FREQ_RANGE.min, FREQ_RANGE.max, 0.50F);

  SetHypercosEffect(HypercosOverlay::MODE2, {hypercosMin, BIG_FREQ_RANGE.max}, amplitudeRange);
}

auto Hypercos::SetMode3RandomParams() -> void
{
  const auto amplitudeRange =
      m_goomRand.ProbabilityOf(PROB_BIG_AMPLITUDE_RANGE) ? BIG_AMPLITUDE_RANGE : AMPLITUDE_RANGE;

  SetHypercosEffect(HypercosOverlay::MODE3, VERY_BIG_FREQ_RANGE, amplitudeRange);
}

auto Hypercos::SetHypercosEffect(const HypercosOverlay overlay,
                                 const IGoomRand::NumberRange<float>& freqRange,
                                 const IGoomRand::NumberRange<float>& amplitudeRange) -> void
{
  const auto xFreq = m_goomRand.GetRandInRange(freqRange);
  const auto yFreq =
      m_goomRand.ProbabilityOf(PROB_FREQ_EQUAL) ? xFreq : m_goomRand.GetRandInRange(freqRange);

  const auto reverse = m_goomRand.ProbabilityOf(PROB_REVERSE);

  const auto xAmplitude = m_goomRand.GetRandInRange(amplitudeRange);
  const auto yAmplitude = m_goomRand.ProbabilityOf(PROB_AMPLITUDE_EQUAL)
                              ? xAmplitude
                              : m_goomRand.GetRandInRange(amplitudeRange);

  SetParams({overlay,
             m_hypercosOverlayWeights.GetRandomWeighted(),
             reverse,
             xFreq,
             yFreq,
             xAmplitude,
             yAmplitude});
}

inline auto Hypercos::GetFreqToUse(const float freq) const -> float
{
  return m_params.reverse ? -freq : +freq;
}

auto Hypercos::GetVelocity(const NormalizedCoords& coords, const NormalizedCoords& velocity) const
    -> NormalizedCoords
{
  const auto xFreqToUse = GetFreqToUse(m_params.xFreq);
  const auto yFreqToUse = GetFreqToUse(m_params.yFreq);

  return velocity + GetVelocity(coords, m_params.effect, xFreqToUse, yFreqToUse);
}

auto Hypercos::GetVelocity(const NormalizedCoords& coords,
                           const HypercosEffect effect,
                           const float xFreqToUse,
                           const float yFreqToUse) const -> NormalizedCoords
{
  auto xVal = 0.0F;
  auto yVal = 0.0F;

  switch (effect)
  {
    case HypercosEffect::NONE:
      break;
    case HypercosEffect::SIN_RECTANGULAR:
      xVal = std::sin(xFreqToUse * coords.GetX());
      yVal = std::sin(yFreqToUse * coords.GetY());
      break;
    case HypercosEffect::COS_RECTANGULAR:
      xVal = std::cos(xFreqToUse * coords.GetX());
      yVal = std::cos(yFreqToUse * coords.GetY());
      break;
    case HypercosEffect::SIN_CURL_SWIRL:
      xVal = std::sin(yFreqToUse * coords.GetY());
      yVal = std::sin(xFreqToUse * coords.GetX());
      break;
    case HypercosEffect::COS_CURL_SWIRL:
      xVal = std::cos(yFreqToUse * coords.GetY());
      yVal = std::cos(xFreqToUse * coords.GetX());
      break;
    case HypercosEffect::SIN_COS_CURL_SWIRL:
      xVal = std::sin(xFreqToUse * coords.GetY());
      yVal = std::cos(yFreqToUse * coords.GetX());
      break;
    case HypercosEffect::COS_SIN_CURL_SWIRL:
      xVal = std::cos(yFreqToUse * coords.GetY());
      yVal = std::sin(xFreqToUse * coords.GetX());
      break;
    case HypercosEffect::SIN_TAN_CURL_SWIRL:
      xVal = std::sin(std::tan(yFreqToUse * coords.GetY()));
      yVal = std::cos(std::tan(xFreqToUse * coords.GetX()));
      break;
    case HypercosEffect::COS_TAN_CURL_SWIRL:
      xVal = std::cos(std::tan(yFreqToUse * coords.GetY()));
      yVal = std::sin(std::tan(xFreqToUse * coords.GetX()));
      break;
    case HypercosEffect::SIN_OF_COS_SWIRL:
      xVal = std::sin(pi * std::cos(yFreqToUse * coords.GetY()));
      yVal = std::cos(pi * std::sin(xFreqToUse * coords.GetX()));
      break;
    case HypercosEffect::COS_OF_SIN_SWIRL:
      xVal = std::cos(pi * std::sin(yFreqToUse * coords.GetY()));
      yVal = std::sin(pi * std::cos(xFreqToUse * coords.GetX()));
      break;
    default:
      throw std::logic_error("Unknown Hypercos effect value");
  }

  //  xVal = std::clamp(std::tan(hypercosFreqY * xVal), -1.0, 1.0);
  //  yVal = std::clamp(std::tan(hypercosFreqX * yVal), -1.0, 1.0);

  xVal *= m_params.xAmplitude;
  yVal *= m_params.yAmplitude;

  return {xVal, yVal};
}

auto Hypercos::GetNameValueParams(const std::string& paramGroup) const -> NameValuePairs
{
  const auto fullParamGroup = GetFullParamGroup({paramGroup, "hypercos"});

  if (m_params.overlay == HypercosOverlay::NONE)
  {
    return {GetPair(fullParamGroup, "overlay", std::string{"None"})};
  }

  return {
      GetPair(fullParamGroup, "overlay", static_cast<uint32_t>(m_params.overlay)),
      GetPair(fullParamGroup, "effect", static_cast<uint32_t>(m_params.effect)),
      GetPair(fullParamGroup, "reverse", m_params.reverse),
      GetPair(fullParamGroup, "Freq", Point2dFlt{m_params.xFreq, m_params.yFreq}),
      GetPair(fullParamGroup, "Amplitude", Point2dFlt{m_params.xAmplitude, m_params.yAmplitude}),
  };
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
