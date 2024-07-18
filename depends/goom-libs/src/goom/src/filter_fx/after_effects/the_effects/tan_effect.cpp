module;

// #undef NO_LOGGING

#include "goom/goom_logger.h"

#include <string>

module Goom.FilterFx.AfterEffects.TheEffects.TanEffect;

import Goom.FilterFx.CommonTypes;
import Goom.Utils.EnumUtils;
import Goom.Utils.NameValuePairs;
import Goom.Utils.Math.GoomRand;
import Goom.Lib.Point2d;

namespace GOOM::FILTER_FX::AFTER_EFFECTS
{

using UTILS::EnumToString;
using UTILS::GetFullParamGroup;
using UTILS::GetPair;
using UTILS::NameValuePairs;
using UTILS::NUM;
using UTILS::MATH::GoomRand;
using UTILS::MATH::NumberRange;

static constexpr auto DEFAULT_TAN_TYPE = TanEffect::TanType::TAN_ONLY;
static constexpr auto DEFAULT_COT_MIX  = 1.1F;
static constexpr auto COT_MIX_RANGE    = NumberRange{0.6F, 1.6F};
static constexpr auto TAN_ONLY_WEIGHT  = 500.0F;
static constexpr auto COT_ONLY_WEIGHT  = 1.0F;
static constexpr auto COT_MIX_WEIGHT   = 50.0F;

static constexpr auto DEFAULT_AMPLITUDE        = 1.0F;
static constexpr auto AMPLITUDE_RANGE          = NumberRange{0.10F, 1.11F};
static constexpr auto PROB_XY_AMPLITUDES_EQUAL = 1.00F;

static constexpr auto DEFAULT_LIMITING_FACTOR = 0.75F;
static constexpr auto LIMITING_FACTOR_RANGE   = NumberRange{0.10F, 0.85F};

TanEffect::TanEffect(const GoomRand& goomRand)
  : m_goomRand{&goomRand},
    m_params{DEFAULT_TAN_TYPE, DEFAULT_COT_MIX, {DEFAULT_AMPLITUDE, DEFAULT_AMPLITUDE},
             DEFAULT_LIMITING_FACTOR},
    m_tanEffectWeights{
      *m_goomRand,
      {
        {TanType::TAN_ONLY, TAN_ONLY_WEIGHT},
        {TanType::COT_ONLY, COT_ONLY_WEIGHT},
        {TanType::COT_MIX,  COT_MIX_WEIGHT},
      }
    }
{
}

auto TanEffect::SetRandomParams() -> void
{
  const auto tanType = m_tanEffectWeights.GetRandomWeighted();
  const auto cotMix  = m_goomRand->GetRandInRange(COT_MIX_RANGE);

  const auto xAmplitude = m_goomRand->GetRandInRange(AMPLITUDE_RANGE);
  const auto yAmplitude = m_goomRand->ProbabilityOf(PROB_XY_AMPLITUDES_EQUAL)
                              ? xAmplitude
                              : m_goomRand->GetRandInRange(AMPLITUDE_RANGE);

  const auto limitingFactor = m_goomRand->GetRandInRange(LIMITING_FACTOR_RANGE);

  LogInfo("tanType = {}, cotMix = {}", EnumToString(tanType), cotMix); // NOLINT
  LogInfo("xAmplitude = {}, yAmplitude = {}", xAmplitude, yAmplitude); // NOLINT
  LogInfo("limitingFactor = {}", limitingFactor); // NOLINT

  SetParams({
      tanType, cotMix, {xAmplitude, yAmplitude},
        limitingFactor
  });
}

auto TanEffect::GetNameValueParams(const std::string& paramGroup) const -> NameValuePairs
{
  const auto fullParamGroup = GetFullParamGroup({paramGroup, "tan effect"});
  return {
      GetPair(fullParamGroup, "tan type", EnumToString(m_params.tanType)),
      GetPair(fullParamGroup, "cot mix", m_params.cotMix),
      GetPair(fullParamGroup, "amplitude", Point2dFlt{m_params.amplitude.x, m_params.amplitude.y}),
      GetPair(fullParamGroup, "limiting factor", m_params.limitingFactor),
  };
}

} // namespace GOOM::FILTER_FX::AFTER_EFFECTS
