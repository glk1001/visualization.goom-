#include "y_only.h"

#include "utils/enumutils.h"
#include "utils/name_value_pairs.h"

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

using UTILS::NameValuePairs;
using UTILS::NUM;
using UTILS::MATH::IGoomRand;

static constexpr auto X_DEFAULT_EFFECT = YOnly::YOnlyEffect::X_SIN_Y_SIN;
static constexpr auto Y_DEFAULT_EFFECT = YOnly::YOnlyEffect::NONE;

static constexpr auto X_DEFAULT_FREQ_FACTOR = 1.0F;
static constexpr auto Y_DEFAULT_FREQ_FACTOR = 10.0F;
static constexpr auto X_FREQ_FACTOR_RANGE = IGoomRand::NumberRange<float>{-50.0F, +50.01F};
static constexpr auto Y_FREQ_FACTOR_RANGE = IGoomRand::NumberRange<float>{-50.0F, +50.01F};

static constexpr auto X_DEFAULT_AMPLITUDE = 10.0F;
static constexpr auto Y_DEFAULT_AMPLITUDE = 10.0F;
static constexpr auto X_AMPLITUDE_RANGE = IGoomRand::NumberRange<float>{0.010F, 1.011F};
static constexpr auto Y_AMPLITUDE_RANGE = IGoomRand::NumberRange<float>{0.010F, 1.011F};

static constexpr auto PROB_Y_ONLY_STRICT = 0.9F;
static constexpr auto PROB_FREQ_EQUAL = 0.9F;
static constexpr auto PROB_AMPLITUDE_EQUAL = 0.9F;

YOnly::YOnly(const IGoomRand& goomRand) noexcept
  : m_goomRand{goomRand}, m_params{X_DEFAULT_EFFECT,      Y_DEFAULT_EFFECT,
                                   X_DEFAULT_FREQ_FACTOR, Y_DEFAULT_FREQ_FACTOR,
                                   X_DEFAULT_AMPLITUDE,   Y_DEFAULT_AMPLITUDE}
{
}

auto YOnly::SetRandomParams() -> void
{
  const auto xEffect = static_cast<YOnlyEffect>(
      m_goomRand.GetRandInRange(static_cast<uint32_t>(YOnlyEffect::NONE) + 1, NUM<YOnlyEffect>));
  const auto yEffect = m_goomRand.ProbabilityOf(PROB_Y_ONLY_STRICT)
                           ? YOnlyEffect::NONE
                           : static_cast<YOnlyEffect>(m_goomRand.GetRandInRange(
                                 static_cast<uint32_t>(YOnlyEffect::NONE) + 1, NUM<YOnlyEffect>));

  const auto xFreqFactor = m_goomRand.GetRandInRange(X_FREQ_FACTOR_RANGE);
  const auto yFreqFactor = m_goomRand.ProbabilityOf(PROB_FREQ_EQUAL)
                               ? xFreqFactor
                               : m_goomRand.GetRandInRange(Y_FREQ_FACTOR_RANGE);

  const auto xAmplitude = m_goomRand.GetRandInRange(X_AMPLITUDE_RANGE);
  const auto yAmplitude = m_goomRand.ProbabilityOf(PROB_AMPLITUDE_EQUAL)
                              ? xAmplitude
                              : m_goomRand.GetRandInRange(Y_AMPLITUDE_RANGE);

  SetParams({xEffect, yEffect, xFreqFactor, yFreqFactor, xAmplitude, yAmplitude});
}

auto YOnly::GetYOnlySpeedMultiplier(const YOnlyEffect effect, const NormalizedCoords& coords) const
    -> float
{
  switch (effect)
  {
    case YOnlyEffect::X_SIN_Y_SIN:
      return std::sin(m_params.xFreqFactor * coords.GetX()) *
             std::sin(m_params.yFreqFactor * coords.GetY());
    case YOnlyEffect::X_SIN_Y_COS:
      return std::sin(m_params.xFreqFactor * coords.GetX()) *
             std::cos(m_params.yFreqFactor * coords.GetY());
    case YOnlyEffect::X_COS_Y_SIN:
      return std::cos(m_params.xFreqFactor * coords.GetX()) *
             std::sin(m_params.yFreqFactor * coords.GetY());
    case YOnlyEffect::X_COS_Y_COS:
      return std::cos(m_params.xFreqFactor * coords.GetX()) *
             std::cos(m_params.yFreqFactor * coords.GetY());
    default:
      throw std::logic_error(
          std20::format("Switch: unhandled case '{}'.", UTILS::EnumToString(effect)));
  }
}

auto YOnly::GetSpeedCoefficientsEffectNameValueParams() const -> NameValuePairs
{
  return NameValuePairs();
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
