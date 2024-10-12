module;

// #undef NO_LOGGING

#include <format>
#include <string>

module Goom.FilterFx.AfterEffects.TheEffects.XYLerpEffect;

import Goom.Utils.EnumUtils;
import Goom.Utils.NameValuePairs;
import Goom.Utils.Math.GoomRand;

namespace GOOM::FILTER_FX::AFTER_EFFECTS
{

using UTILS::EnumToString;
using UTILS::GetFullParamGroup;
using UTILS::GetPair;
using UTILS::NameValuePairs;
using UTILS::MATH::GoomRand;
using UTILS::MATH::NumberRange;

static constexpr auto T_FREQ_RANGE     = NumberRange{1.0F, 10.0F};
static constexpr auto PROB_FLIP_Y_SIGN = 0.5F;
static constexpr auto PROB_FLIP_XY     = 0.5F;

static constexpr auto MODE0_WEIGHT = 10.0F;
static constexpr auto MODE1_WEIGHT = 10.0F;
static constexpr auto MODE2_WEIGHT = 10.0F;
static constexpr auto MODE3_WEIGHT = 10000000000.0F;

XYLerpEffect::XYLerpEffect(const GoomRand& goomRand)
  : m_goomRand{&goomRand},
    m_modeWeights{
        *m_goomRand,
        {
            {.key=Modes::MODE0, .weight=MODE0_WEIGHT},
            {.key=Modes::MODE1, .weight=MODE1_WEIGHT},
            {.key=Modes::MODE2, .weight=MODE2_WEIGHT},
            {.key=Modes::MODE3, .weight=MODE3_WEIGHT},
        }
    },
    m_params{GetRandomParams()}
{
}

auto XYLerpEffect::GetRandomParams() const noexcept -> Params
{
  const auto mode   = m_modeWeights.GetRandomWeighted();
  const auto tFreq  = m_goomRand->GetRandInRange<T_FREQ_RANGE>();
  const auto ySign  = m_goomRand->ProbabilityOf<PROB_FLIP_Y_SIGN>() ? -1.0F : +1.0F;
  const auto flipXY = m_goomRand->ProbabilityOf(GetFlipYProbability(mode));

  return {.mode = mode, .tFreq = tFreq, .ySign = ySign, .flipXY = flipXY};
}

inline auto XYLerpEffect::GetFlipYProbability(const Modes mode) -> float
{
  if ((mode == Modes::MODE0) or (mode == Modes::MODE1))
  {
    return 0.0F;
  }

  return PROB_FLIP_XY;
}

auto XYLerpEffect::GetNameValueParams(const std::string& paramGroup) const -> NameValuePairs
{
  const auto fullParamGroup = GetFullParamGroup({paramGroup, "xy lerp effect"});
  return {
      GetPair(fullParamGroup,
              "params",
              std::format("{}, {:.2f}, {:.1f}, {}",
                          EnumToString(m_params.mode),
                          m_params.tFreq,
                          m_params.ySign,
                          m_params.flipXY)),
  };
}

} // namespace GOOM::FILTER_FX::AFTER_EFFECTS
