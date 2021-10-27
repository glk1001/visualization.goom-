#include "tan_effect.h"

#include "utils/goomrand.h"
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
using UTILS::NameValuePairs;
using UTILS::NumberRange;
using UTILS::ProbabilityOf;

constexpr float DEFAULT_AMPLITUDE = 1.0F;
constexpr NumberRange<float> AMPLITUDE_RANGE = {0.10F, 1.11F};
constexpr float PROB_XY_AMPLITUDES_EQUAL = 0.90F;

constexpr float DEFAULT_LIMITING_FACTOR = 0.75F;
constexpr NumberRange<float> LIMITING_FACTOR_RANGE = {0.10F, 0.85F};

TanEffect::TanEffect() noexcept
  : m_params{DEFAULT_AMPLITUDE, DEFAULT_AMPLITUDE, DEFAULT_LIMITING_FACTOR}
{
}

void TanEffect::SetRandomParams()
{
  m_params.xAmplitude = GetRandInRange(AMPLITUDE_RANGE);
  m_params.yAmplitude = ProbabilityOf(PROB_XY_AMPLITUDES_EQUAL) ? m_params.xAmplitude
                                                                : GetRandInRange(AMPLITUDE_RANGE);
  m_params.limitingFactor = GetRandInRange(LIMITING_FACTOR_RANGE);
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

#if __cplusplus <= 201402L
} // namespace FILTERS
} // namespace GOOM
#else
} // namespace GOOM::FILTERS
#endif
