#include "filter_hypercos.h"

#include "filter_data.h"
#include "goomutils/enumutils.h"
#include "goomutils/goomrand.h"

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

using UTILS::GetRandInRange;
using UTILS::NUM;
using UTILS::NumberRange;
using UTILS::ProbabilityOf;

// Hypercos:
// applique une surcouche de hypercos effect
// applies an overlay of hypercos effect
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
constexpr float PROB_BIG_AMPLITUDE_RANGE = 0.8F;

constexpr Hypercos::Params DEFAULT_PARAMS{DEFAULT_EFFECT, DEFAULT_REVERSE,     X_DEFAULT_FREQ,
                                          Y_DEFAULT_FREQ, X_DEFAULT_AMPLITUDE, Y_DEFAULT_AMPLITUDE};

Hypercos::Hypercos() noexcept : m_params{DEFAULT_PARAMS}
{
}

void Hypercos::SetDefaultParams()
{
  m_params = DEFAULT_PARAMS;
}

void Hypercos::SetMode0RandomParams()
{
  const float hypercosMax = stdnew::lerp(FREQ_RANGE.min, FREQ_RANGE.max, 0.15F);
  SetHypercosEffect({FREQ_RANGE.min, hypercosMax}, AMPLITUDE_RANGE);
}

void Hypercos::SetMode1RandomParams()
{
  const float hypercosMin = stdnew::lerp(FREQ_RANGE.min, FREQ_RANGE.max, 0.20F);
  SetHypercosEffect({hypercosMin, FREQ_RANGE.max}, AMPLITUDE_RANGE);
}

void Hypercos::SetMode2RandomParams()
{
  const NumberRange<float> amplitudeRange =
      ProbabilityOf(PROB_BIG_AMPLITUDE_RANGE) ? BIG_AMPLITUDE_RANGE : AMPLITUDE_RANGE;

  const float hypercosMin = stdnew::lerp(FREQ_RANGE.min, FREQ_RANGE.max, 0.50F);

  SetHypercosEffect({hypercosMin, BIG_FREQ_RANGE.max}, amplitudeRange);
}

void Hypercos::SetMode3RandomParams()
{
  const NumberRange<float> amplitudeRange =
      ProbabilityOf(PROB_BIG_AMPLITUDE_RANGE) ? BIG_AMPLITUDE_RANGE : AMPLITUDE_RANGE;

  SetHypercosEffect(VERY_BIG_FREQ_RANGE, amplitudeRange);
}

void Hypercos::SetHypercosEffect(const UTILS::NumberRange<float>& freqRange,
                                 const UTILS::NumberRange<float>& amplitudeRange)
{
  m_params.effect = static_cast<HypercosEffect>(
      GetRandInRange(static_cast<uint32_t>(HypercosEffect::NONE) + 1, NUM<HypercosEffect>));

  m_params.xFreq = GetRandInRange(freqRange);
  m_params.yFreq = ProbabilityOf(PROB_FREQ_EQUAL) ? m_params.xFreq : GetRandInRange(freqRange);

  m_params.reverse = ProbabilityOf(PROB_REVERSE);

  m_params.xAmplitude = GetRandInRange(amplitudeRange);
  m_params.yAmplitude =
      ProbabilityOf(PROB_AMPLITUDE_EQUAL) ? m_params.xAmplitude : GetRandInRange(amplitudeRange);
}

auto Hypercos::GetVelocity(const NormalizedCoords& coords) const -> NormalizedCoords
{
  const float xFreqToUse = m_params.reverse ? -m_params.xFreq : +m_params.xFreq;
  const float yFreqToUse = m_params.reverse ? -m_params.yFreq : +m_params.yFreq;

  float xVal = 0.0;
  float yVal = 0.0;
  switch (m_params.effect)
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
    default:
      throw std::logic_error("Unknown Hypercos effect value");
  }

  //  xVal = stdnew::clamp(std::tan(hypercosFreqY * xVal), -1.0, 1.0);
  //  yVal = stdnew::clamp(std::tan(hypercosFreqX * yVal), -1.0, 1.0);

  return {m_params.xAmplitude * xVal, m_params.yAmplitude * yVal};
}

#if __cplusplus <= 201402L
} // namespace FILTERS
} // namespace GOOM
#else
} // namespace GOOM::FILTERS
#endif
