#include "amulet.h"

#include "utils/name_value_pairs.h"

namespace GOOM::VISUAL_FX::FILTERS
{

using UTILS::NameValuePairs;
using UTILS::MATH::IGoomRand;

static constexpr float DEFAULT_AMPLITUDE = 1.0F;
static constexpr IGoomRand::NumberRange<float> AMPLITUDE_RANGE = {0.1F, 1.51F};

static constexpr float PROB_XY_AMPLITUDES_EQUAL = 0.98F;

Amulet::Amulet(const IGoomRand& goomRand) noexcept
  : m_goomRand{goomRand}, m_params{DEFAULT_AMPLITUDE, DEFAULT_AMPLITUDE}
{
}

void Amulet::SetRandomParams()
{
  const float xAmplitude = m_goomRand.GetRandInRange(AMPLITUDE_RANGE);
  const float yAmplitude = m_goomRand.ProbabilityOf(PROB_XY_AMPLITUDES_EQUAL)
                               ? xAmplitude
                               : m_goomRand.GetRandInRange(AMPLITUDE_RANGE);

  SetParams({xAmplitude, yAmplitude});
}

auto Amulet::GetSpeedCoefficientsEffectNameValueParams() const -> NameValuePairs
{
  return NameValuePairs();
}

} // namespace GOOM::VISUAL_FX::FILTERS
