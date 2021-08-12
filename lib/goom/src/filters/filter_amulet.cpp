#include "filter_amulet.h"

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
using UTILS::NumberRange;
using UTILS::ProbabilityOf;

constexpr float DEFAULT_AMPLITUDE = 3.5F;
constexpr NumberRange<float> AMPLITUDE_RANGE = {2.0F, 5.0F};

constexpr float PROB_XY_AMPLITUDES_EQUAL = 0.9F;

Amulet::Amulet() noexcept : m_params{DEFAULT_AMPLITUDE, DEFAULT_AMPLITUDE}
{
}

void Amulet::SetRandomParams()
{
  m_params.xAmplitude = GetRandInRange(AMPLITUDE_RANGE);
  m_params.yAmplitude = ProbabilityOf(PROB_XY_AMPLITUDES_EQUAL) ? m_params.xAmplitude
                                                                : GetRandInRange(AMPLITUDE_RANGE);
}

#if __cplusplus <= 201402L
} // namespace FILTERS
} // namespace GOOM
#else
} // namespace GOOM::FILTERS
#endif
