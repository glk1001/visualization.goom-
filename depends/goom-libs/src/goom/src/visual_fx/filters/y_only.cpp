#include "y_only.h"

#include "utils/enumutils.h"
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

using UTILS::IGoomRand;
using UTILS::NameValuePairs;
using UTILS::NUM;

constexpr YOnly::YOnlyEffect X_DEFAULT_EFFECT = YOnly::YOnlyEffect::XSIN_YSIN;
constexpr YOnly::YOnlyEffect Y_DEFAULT_EFFECT = YOnly::YOnlyEffect::NONE;

constexpr float X_DEFAULT_FREQ_FACTOR = 1.0F;
constexpr float Y_DEFAULT_FREQ_FACTOR = 10.0F;
constexpr IGoomRand::NumberRange<float> X_FREQ_FACTOR_RANGE = {-50.0F, +50.01F};
constexpr IGoomRand::NumberRange<float> Y_FREQ_FACTOR_RANGE = {-50.0F, +50.01F};

constexpr float X_DEFAULT_AMPLITUDE = 10.0F;
constexpr float Y_DEFAULT_AMPLITUDE = 10.0F;
constexpr IGoomRand::NumberRange<float> X_AMPLITUDE_RANGE = {0.010F, 1.011F};
constexpr IGoomRand::NumberRange<float> Y_AMPLITUDE_RANGE = {0.010F, 1.011F};

constexpr float PROB_Y_ONLY_STRICT = 0.9F;
constexpr float PROB_FREQ_EQUAL = 0.9F;
constexpr float PROB_AMPLITUDE_EQUAL = 0.9F;

YOnly::YOnly(IGoomRand& goomRand) noexcept
  : m_goomRand{goomRand}, m_params{X_DEFAULT_EFFECT,      Y_DEFAULT_EFFECT,
                                   X_DEFAULT_FREQ_FACTOR, Y_DEFAULT_FREQ_FACTOR,
                                   X_DEFAULT_AMPLITUDE,   Y_DEFAULT_AMPLITUDE}
{
}

void YOnly::SetRandomParams()
{
  m_params.xEffect = static_cast<YOnlyEffect>(
      m_goomRand.GetRandInRange(static_cast<uint32_t>(YOnlyEffect::NONE) + 1, NUM<YOnlyEffect>));
  m_params.yEffect = m_goomRand.ProbabilityOf(PROB_Y_ONLY_STRICT)
                         ? YOnlyEffect::NONE
                         : static_cast<YOnlyEffect>(m_goomRand.GetRandInRange(
                               static_cast<uint32_t>(YOnlyEffect::NONE) + 1, NUM<YOnlyEffect>));

  m_params.xFreqFactor = m_goomRand.GetRandInRange(X_FREQ_FACTOR_RANGE);
  m_params.yFreqFactor = m_goomRand.ProbabilityOf(PROB_FREQ_EQUAL)
                             ? m_params.xFreqFactor
                             : m_goomRand.GetRandInRange(Y_FREQ_FACTOR_RANGE);

  m_params.xAmplitude = m_goomRand.GetRandInRange(X_AMPLITUDE_RANGE);
  m_params.yAmplitude = m_goomRand.ProbabilityOf(PROB_AMPLITUDE_EQUAL)
                            ? m_params.xAmplitude
                            : m_goomRand.GetRandInRange(Y_AMPLITUDE_RANGE);
}

auto YOnly::GetSpeedCoefficientsEffectNameValueParams() const -> NameValuePairs
{
  return NameValuePairs();
}

#if __cplusplus <= 201402L
} // namespace FILTERS
} // namespace GOOM
#else
} // namespace GOOM::FILTERS
#endif
