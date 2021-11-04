#include "hypercos.h"

#include "utils/enumutils.h"
#include "utils/randutils.h"
#include "utils/mathutils.h"
#include "utils/name_value_pairs.h"

#undef NDEBUG
#include <cassert>

#if __cplusplus <= 201402L
namespace GOOM
{
namespace FILTERS
{
#else
namespace GOOM::FILTERS
{
#endif

using UTILS::GetFullParamGroup;
using UTILS::GetPair;
using UTILS::GetRandInRange;
using UTILS::m_pi;
using UTILS::NameValuePairs;
using UTILS::NUM;
using UTILS::NumberRange;
using UTILS::ProbabilityOf;
using UTILS::Weights;

// Hypercos:
// applique une surcouche de hypercos effect
// applies an overlay of hypercos effect
constexpr HypercosOverlay DEFAULT_OVERLAY = HypercosOverlay::NONE;
constexpr Hypercos::HypercosEffect DEFAULT_EFFECT = Hypercos::HypercosEffect::NONE;
constexpr bool DEFAULT_REVERSE = false;

constexpr float X_DEFAULT_FREQ = 10.0F;
constexpr float Y_DEFAULT_FREQ = 10.0F;
constexpr NumberRange<float> FREQ_RANGE = {5.0F, 100.0F};
constexpr NumberRange<float> BIG_FREQ_RANGE = {5.0F, 500.0F};
constexpr NumberRange<float> VERY_BIG_FREQ_RANGE = {30000.0F, 50000.0F};

constexpr float X_DEFAULT_AMPLITUDE = 1.0F / 120.0F;
constexpr float Y_DEFAULT_AMPLITUDE = 1.0F / 120.0F;
constexpr NumberRange<float> AMPLITUDE_RANGE = {0.1F * X_DEFAULT_AMPLITUDE,
                                                1.1F * X_DEFAULT_AMPLITUDE};
constexpr NumberRange<float> BIG_AMPLITUDE_RANGE = {0.1F * X_DEFAULT_AMPLITUDE,
                                                    10.1F * X_DEFAULT_AMPLITUDE};

constexpr float PROB_FREQ_EQUAL = 0.5F;
constexpr float PROB_REVERSE = 0.5F;
constexpr float PROB_AMPLITUDE_EQUAL = 0.5F;
constexpr float PROB_BIG_AMPLITUDE_RANGE = 0.2F;

constexpr Hypercos::Params DEFAULT_PARAMS{DEFAULT_OVERLAY,    DEFAULT_EFFECT, DEFAULT_REVERSE,
                                          X_DEFAULT_FREQ,     Y_DEFAULT_FREQ, X_DEFAULT_AMPLITUDE,
                                          Y_DEFAULT_AMPLITUDE};

Hypercos::Hypercos() noexcept : m_params{DEFAULT_PARAMS}
{
}

void Hypercos::SetDefaultParams()
{
  m_params = DEFAULT_PARAMS;
}

void Hypercos::SetMode0RandomParams()
{
  m_params.overlay = HypercosOverlay::MODE0;

  const float hypercosMax = stdnew::lerp(FREQ_RANGE.min, FREQ_RANGE.max, 0.15F);
  SetHypercosEffect({FREQ_RANGE.min, hypercosMax}, AMPLITUDE_RANGE);
}

void Hypercos::SetMode1RandomParams()
{
  m_params.overlay = HypercosOverlay::MODE1;

  const float hypercosMin = stdnew::lerp(FREQ_RANGE.min, FREQ_RANGE.max, 0.20F);
  SetHypercosEffect({hypercosMin, FREQ_RANGE.max}, AMPLITUDE_RANGE);
}

void Hypercos::SetMode2RandomParams()
{
  m_params.overlay = HypercosOverlay::MODE2;

  const NumberRange<float> amplitudeRange =
      ProbabilityOf(PROB_BIG_AMPLITUDE_RANGE) ? BIG_AMPLITUDE_RANGE : AMPLITUDE_RANGE;

  const float hypercosMin = stdnew::lerp(FREQ_RANGE.min, FREQ_RANGE.max, 0.50F);

  SetHypercosEffect({hypercosMin, BIG_FREQ_RANGE.max}, amplitudeRange);
}

void Hypercos::SetMode3RandomParams()
{
  m_params.overlay = HypercosOverlay::MODE3;

  const NumberRange<float> amplitudeRange =
      ProbabilityOf(PROB_BIG_AMPLITUDE_RANGE) ? BIG_AMPLITUDE_RANGE : AMPLITUDE_RANGE;

  SetHypercosEffect(VERY_BIG_FREQ_RANGE, amplitudeRange);
}

void Hypercos::SetHypercosEffect(const UTILS::NumberRange<float>& freqRange,
                                 const UTILS::NumberRange<float>& amplitudeRange)
{
  // clang-format off
  // @formatter:off
  static const Weights<HypercosEffect> s_hypercosOverlayWeights{{
      { HypercosEffect::NONE,                0 },
      { HypercosEffect::SIN_CURL_SWIRL,     15 },
      { HypercosEffect::COS_CURL_SWIRL,     15 },
      { HypercosEffect::SIN_COS_CURL_SWIRL, 15 },
      { HypercosEffect::COS_SIN_CURL_SWIRL, 15 },
      { HypercosEffect::SIN_TAN_CURL_SWIRL,  5 },
      { HypercosEffect::COS_TAN_CURL_SWIRL,  5 },
      { HypercosEffect::SIN_RECTANGULAR,     5 },
      { HypercosEffect::COS_RECTANGULAR,     5 },
      { HypercosEffect::SIN_OF_COS_SWIRL,   15 },
      { HypercosEffect::COS_OF_SIN_SWIRL,   15 },
  }};
  // @formatter:on
  // clang-format on
  m_params.effect = s_hypercosOverlayWeights.GetRandomWeighted();

  m_params.xFreq = GetRandInRange(freqRange);
  m_params.yFreq = ProbabilityOf(PROB_FREQ_EQUAL) ? m_params.xFreq : GetRandInRange(freqRange);

  m_params.reverse = ProbabilityOf(PROB_REVERSE);

  m_params.xAmplitude = GetRandInRange(amplitudeRange);
  m_params.yAmplitude =
      ProbabilityOf(PROB_AMPLITUDE_EQUAL) ? m_params.xAmplitude : GetRandInRange(amplitudeRange);
}

inline auto Hypercos::GetFreqToUse(const float freq) const -> float
{
  return m_params.reverse ? -freq : +freq;
}

auto Hypercos::GetVelocity(const NormalizedCoords& coords) const -> NormalizedCoords
{
  //  if (std::fabs(std::sin(10.0F * coords.GetX())) < 0.5F && std::fabs(std::cos(10.0F * coords.GetY())) < 0.5F)
  //  if (UTILS::Sq(coords.GetX() - 0.5F) + UTILS::Sq(coords.GetY() - 0.5F) < UTILS::Sq(0.2F) ||
  //      UTILS::Sq(coords.GetX() + 0.5F) + UTILS::Sq(coords.GetY() + 0.5F) < UTILS::Sq(0.2F) ||
  //      UTILS::Sq(coords.GetX() - 0.5F) + UTILS::Sq(coords.GetY() + 0.5F) < UTILS::Sq(0.2F) ||
  //      UTILS::Sq(coords.GetX() + 0.5F) + UTILS::Sq(coords.GetY() - 0.5F) < UTILS::Sq(0.2F))
  //  {
  //    return {-0.05F, -0.05F};
  //  }

  const float xFreqToUse = GetFreqToUse(m_params.xFreq);
  const float yFreqToUse = GetFreqToUse(m_params.yFreq);

  //  if (UTILS::Sq(coords.GetX() - 0.5F) + UTILS::Sq(coords.GetY() - 0.5F) < UTILS::Sq(0.2F) ||
  //    UTILS::Sq(coords.GetX() + 0.5F) + UTILS::Sq(coords.GetY() + 0.5F) < UTILS::Sq(0.2F) ||
  //    UTILS::Sq(coords.GetX() - 0.5F) + UTILS::Sq(coords.GetY() + 0.5F) < UTILS::Sq(0.2F) ||
  //    UTILS::Sq(coords.GetX() + 0.5F) + UTILS::Sq(coords.GetY() - 0.5F) < UTILS::Sq(0.2F))
  //    {
  //      return GetVelocity(coords, HypercosEffect::COS_TAN_CURL_SWIRL, xFreqToUse, yFreqToUse);
  //    }

  return GetVelocity(coords, m_params.effect, xFreqToUse, yFreqToUse);
}

auto Hypercos::GetVelocity(const NormalizedCoords& coords,
                           const HypercosEffect effect,
                           const float xFreqToUse,
                           const float yFreqToUse) const -> NormalizedCoords
{
  float xVal = 0.0;
  float yVal = 0.0;

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
      xVal = std::sin(m_pi * std::cos(yFreqToUse * coords.GetY()));
      yVal = std::cos(m_pi * std::sin(xFreqToUse * coords.GetX()));
      break;
    case HypercosEffect::COS_OF_SIN_SWIRL:
      xVal = std::cos(m_pi * std::sin(yFreqToUse * coords.GetY()));
      yVal = std::sin(m_pi * std::cos(xFreqToUse * coords.GetX()));
      break;
    default:
      throw std::logic_error("Unknown Hypercos effect value");
  }

  //  xVal = stdnew::clamp(std::tan(hypercosFreqY * xVal), -1.0, 1.0);
  //  yVal = stdnew::clamp(std::tan(hypercosFreqX * yVal), -1.0, 1.0);

  return {m_params.xAmplitude * xVal, m_params.yAmplitude * yVal};
}

auto Hypercos::GetNameValueParams(const std::string& paramGroup) const -> NameValuePairs
{
  const std::string fullParamGroup = GetFullParamGroup({paramGroup, "hypercos"});

  if (m_params.overlay == HypercosOverlay::NONE)
  {
    return {GetPair(fullParamGroup, "overlay", std::string{"None"})};
  }

  return {
      GetPair(fullParamGroup, "overlay", static_cast<uint32_t>(m_params.overlay)),
      GetPair(fullParamGroup, "effect", static_cast<uint32_t>(m_params.effect)),
      GetPair(fullParamGroup, "reverse", m_params.reverse),
      GetPair(fullParamGroup, "xFreq", m_params.xFreq),
      GetPair(fullParamGroup, "yFreq", m_params.yFreq),
      GetPair(fullParamGroup, "xAmplitude", m_params.xAmplitude),
      GetPair(fullParamGroup, "yAmplitude", m_params.yAmplitude),
  };
}

#if __cplusplus <= 201402L
} // namespace FILTERS
} // namespace GOOM
#else
} // namespace GOOM::FILTERS
#endif
