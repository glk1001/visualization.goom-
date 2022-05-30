#include "tan_effect.h"

// #undef NO_LOGGING

#include "goom/logging.h"
#include "utils/enumutils.h"
#include "utils/name_value_pairs.h"

namespace GOOM::VISUAL_FX::FILTERS
{

using UTILS::EnumToString;
using UTILS::GetFullParamGroup;
using UTILS::GetPair;
using UTILS::Logging;
using UTILS::NameValuePairs;
using UTILS::NUM;
using UTILS::MATH::IGoomRand;
using UTILS::MATH::Weights;

static constexpr TanEffect::TanType DEFAULT_TAN_TYPE = TanEffect::TanType::TAN_ONLY;
static constexpr float DEFAULT_COT_MIX = 1.1F;
static constexpr IGoomRand::NumberRange<float> COT_MIX_RANGE = {0.6F, 1.6F};
static constexpr float TAN_ONLY_WEIGHT = 500.0F;
static constexpr float COT_ONLY_WEIGHT = 1.0F;
static constexpr float COT_MIX_WEIGHT = 50.0F;

static constexpr float DEFAULT_AMPLITUDE = 1.0F;
static constexpr IGoomRand::NumberRange<float> AMPLITUDE_RANGE = {0.10F, 1.11F};
static constexpr float PROB_XY_AMPLITUDES_EQUAL = 1.00F;

static constexpr float DEFAULT_LIMITING_FACTOR = 0.75F;
static constexpr IGoomRand::NumberRange<float> LIMITING_FACTOR_RANGE = {0.10F, 0.85F};

TanEffect::TanEffect(const IGoomRand& goomRand)
  : m_goomRand{goomRand},
    m_params{DEFAULT_TAN_TYPE, DEFAULT_COT_MIX, DEFAULT_AMPLITUDE, DEFAULT_AMPLITUDE,
             DEFAULT_LIMITING_FACTOR},
    // clang-format off
    m_tanEffectWeights{
      m_goomRand,
      {
        {TanType::TAN_ONLY, TAN_ONLY_WEIGHT},
        {TanType::COT_ONLY, COT_ONLY_WEIGHT},
        {TanType::COT_MIX,  COT_MIX_WEIGHT},
      }
    }
// clang-format on
{
}

void TanEffect::SetRandomParams()
{
  const TanType tanType = m_tanEffectWeights.GetRandomWeighted();
  const float cotMix = m_goomRand.GetRandInRange(COT_MIX_RANGE);

  const float xAmplitude = m_goomRand.GetRandInRange(AMPLITUDE_RANGE);
  const float yAmplitude = m_goomRand.ProbabilityOf(PROB_XY_AMPLITUDES_EQUAL)
                               ? xAmplitude
                               : m_goomRand.GetRandInRange(AMPLITUDE_RANGE);
  const float limitingFactor = m_goomRand.GetRandInRange(LIMITING_FACTOR_RANGE);

  LogInfo("tanType = {}, cotMix = {}", EnumToString(tanType), cotMix);
  LogInfo("xAmplitude = {}, yAmplitude = {}", xAmplitude, yAmplitude);
  LogInfo("limitingFactor = {}", limitingFactor);

  SetParams({tanType, cotMix, xAmplitude, yAmplitude, limitingFactor});
}

auto TanEffect::GetNameValueParams(const std::string& paramGroup) const -> NameValuePairs
{
  const std::string fullParamGroup = GetFullParamGroup({paramGroup, "tan effect"});
  return {
      GetPair(fullParamGroup, "tan type", EnumToString(m_params.tanType)),
      GetPair(fullParamGroup, "cot mix", m_params.cotMix),
      GetPair(fullParamGroup, "amplitude", Point2dFlt{m_params.xAmplitude, m_params.yAmplitude}),
      GetPair(fullParamGroup, "limiting factor", m_params.limitingFactor),
  };
}

} // namespace GOOM::VISUAL_FX::FILTERS
