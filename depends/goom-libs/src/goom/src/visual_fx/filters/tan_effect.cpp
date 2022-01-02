#include "tan_effect.h"

#include "utils/name_value_pairs.h"

namespace GOOM::VISUAL_FX::FILTERS
{

using UTILS::GetFullParamGroup;
using UTILS::GetPair;
using UTILS::IGoomRand;
using UTILS::NameValuePairs;

constexpr float DEFAULT_AMPLITUDE = 1.0F;
constexpr IGoomRand::NumberRange<float> AMPLITUDE_RANGE = {0.10F, 1.11F};
constexpr float PROB_XY_AMPLITUDES_EQUAL = 0.90F;

constexpr float DEFAULT_LIMITING_FACTOR = 0.75F;
constexpr IGoomRand::NumberRange<float> LIMITING_FACTOR_RANGE = {0.10F, 0.85F};

TanEffect::TanEffect(const IGoomRand& goomRand) noexcept
  : m_goomRand{goomRand}, m_params{DEFAULT_AMPLITUDE, DEFAULT_AMPLITUDE, DEFAULT_LIMITING_FACTOR}
{
}

void TanEffect::SetRandomParams()
{
  const float xAmplitude = m_goomRand.GetRandInRange(AMPLITUDE_RANGE);
  const float yAmplitude = m_goomRand.ProbabilityOf(PROB_XY_AMPLITUDES_EQUAL)
                               ? xAmplitude
                               : m_goomRand.GetRandInRange(AMPLITUDE_RANGE);
  const float limitingFactor = m_goomRand.GetRandInRange(LIMITING_FACTOR_RANGE);

  SetParams({xAmplitude, yAmplitude, limitingFactor});
}

auto TanEffect::GetNameValueParams(const std::string& paramGroup) const -> NameValuePairs
{
  const std::string fullParamGroup = GetFullParamGroup({paramGroup, "tan effect"});
  return {
      GetPair(fullParamGroup, "x amplitude", m_params.xAmplitude),
      GetPair(fullParamGroup, "y amplitude", m_params.yAmplitude),
      GetPair(fullParamGroup, "limiting factor", m_params.limitingFactor),
  };
}

} // namespace GOOM::VISUAL_FX::FILTERS
