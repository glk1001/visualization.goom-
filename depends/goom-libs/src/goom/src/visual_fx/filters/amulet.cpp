#include "amulet.h"

#include "utils/name_value_pairs.h"

#undef NDEBUG
#include <cassert>

#if __cplusplus <= 201402L
namespace GOOM
{
namespace VISUAL_FX
{
namespace FILTERS
{
#else
namespace GOOM::VISUAL_FX::FILTERS
{
#endif

using UTILS::IGoomRand;
using UTILS::NameValuePairs;

constexpr float DEFAULT_AMPLITUDE = 1.0F;
constexpr IGoomRand::NumberRange<float> AMPLITUDE_RANGE = {0.1F, 1.51F};

constexpr float PROB_XY_AMPLITUDES_EQUAL = 0.98F;

Amulet::Amulet(IGoomRand& goomRand) noexcept
  : m_goomRand{goomRand}, m_params{DEFAULT_AMPLITUDE, DEFAULT_AMPLITUDE}
{
}

void Amulet::SetRandomParams()
{
  m_params.xAmplitude = m_goomRand.GetRandInRange(AMPLITUDE_RANGE);
  m_params.yAmplitude = m_goomRand.ProbabilityOf(PROB_XY_AMPLITUDES_EQUAL)
                            ? m_params.xAmplitude
                            : m_goomRand.GetRandInRange(AMPLITUDE_RANGE);
}

auto Amulet::GetSpeedCoefficientsEffectNameValueParams() const -> NameValuePairs
{
  return NameValuePairs();
}

#if __cplusplus <= 201402L
} // namespace FILTERS
} // namespace VISUAL_FX
} // namespace GOOM
#else
} // namespace GOOM::VISUAL_FX::FILTERS
#endif
