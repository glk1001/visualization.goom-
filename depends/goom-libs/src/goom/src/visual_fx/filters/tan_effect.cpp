#include "tan_effect.h"

#include "utils/enumutils.h"
#include "utils/name_value_pairs.h"

namespace GOOM::VISUAL_FX::FILTERS
{

using UTILS::EnumToString;
using UTILS::GetFullParamGroup;
using UTILS::GetPair;
using UTILS::IGoomRand;
using UTILS::NameValuePairs;
using UTILS::NUM;
using UTILS::Weights;

constexpr TanEffect::TanType DEFAULT_TAN_TYPE = TanEffect::TanType::TAN_ONLY;
constexpr float DEFAULT_COT_MIX = 1.1F;
constexpr IGoomRand::NumberRange<float> COT_MIX_RANGE = {0.6F, 1.6F};
constexpr float TAN_ONLY_WEIGHT = 100.0F;
constexpr float COT_ONLY_WEIGHT = 5.0F;
constexpr float COT_MIX_WEIGHT = 1.0F;

constexpr float DEFAULT_AMPLITUDE = 1.0F;
constexpr IGoomRand::NumberRange<float> AMPLITUDE_RANGE = {0.10F, 1.11F};
constexpr float PROB_XY_AMPLITUDES_EQUAL = 1.00F;

constexpr float DEFAULT_LIMITING_FACTOR = 0.75F;
constexpr IGoomRand::NumberRange<float> LIMITING_FACTOR_RANGE = {0.10F, 0.85F};

TanEffect::TanEffect(const IGoomRand& goomRand) noexcept
  : m_goomRand{goomRand},
    m_params{DEFAULT_TAN_TYPE, DEFAULT_COT_MIX, DEFAULT_AMPLITUDE, DEFAULT_AMPLITUDE,
             DEFAULT_LIMITING_FACTOR}
{
}

void TanEffect::SetRandomParams()
{
  // clang-format off
  static const Weights<TanType> s_TAN_EFFECT_WEIGHTS{
      m_goomRand,
      {
        {TanType::TAN_ONLY, TAN_ONLY_WEIGHT},
        {TanType::COT_ONLY, COT_ONLY_WEIGHT},
        {TanType::COT_MIX,  COT_MIX_WEIGHT},
      }
  };
  // clang-format on

  const TanType tanType = s_TAN_EFFECT_WEIGHTS.GetRandomWeighted();
  const float cotMix = m_goomRand.GetRandInRange(COT_MIX_RANGE);

  const float xAmplitude = m_goomRand.GetRandInRange(AMPLITUDE_RANGE);
  const float yAmplitude = m_goomRand.ProbabilityOf(PROB_XY_AMPLITUDES_EQUAL)
                               ? xAmplitude
                               : m_goomRand.GetRandInRange(AMPLITUDE_RANGE);
  const float limitingFactor = m_goomRand.GetRandInRange(LIMITING_FACTOR_RANGE);

  SetParams({tanType, cotMix, xAmplitude, yAmplitude, limitingFactor});
}

auto TanEffect::GetNameValueParams(const std::string& paramGroup) const -> NameValuePairs
{
  const std::string fullParamGroup = GetFullParamGroup({paramGroup, "tan effect"});
  return {
      GetPair(fullParamGroup, "tan type", EnumToString(m_params.tanType)),
      GetPair(fullParamGroup, "cot mix", m_params.cotMix),
      GetPair(fullParamGroup, "x amplitude", m_params.xAmplitude),
      GetPair(fullParamGroup, "y amplitude", m_params.yAmplitude),
      GetPair(fullParamGroup, "limiting factor", m_params.limitingFactor),
  };
}

} // namespace GOOM::VISUAL_FX::FILTERS
