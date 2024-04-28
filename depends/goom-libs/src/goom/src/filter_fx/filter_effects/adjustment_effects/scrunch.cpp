#include "scrunch.h"

#include "filter_fx/common_types.h"
#include "goom/goom_config.h"
#include "utils/math/goom_rand_base.h"

import Goom.Utils;

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

using UTILS::NameValuePairs;
using UTILS::MATH::IGoomRand;

static constexpr auto X_DEFAULT_AMPLITUDE = 0.1F;
static constexpr auto Y_DEFAULT_AMPLITUDE = 1.0F;
static constexpr auto AMPLITUDE_RANGE     = AmplitudeRange{
        {0.05F, 0.20F},
        {0.50F, 5.00F},
};

Scrunch::Scrunch(const IGoomRand& goomRand) noexcept
  : m_goomRand{&goomRand}, m_params{X_DEFAULT_AMPLITUDE, Y_DEFAULT_AMPLITUDE}
{
}

auto Scrunch::SetRandomParams() noexcept -> void
{
  const auto xAmplitude = m_goomRand->GetRandInRange(AMPLITUDE_RANGE.xRange);
  const auto yAmplitude = m_goomRand->GetRandInRange(AMPLITUDE_RANGE.yRange);

  SetParams({xAmplitude, yAmplitude});
}

auto Scrunch::GetZoomAdjustmentEffectNameValueParams() const noexcept -> NameValuePairs
{
  return NameValuePairs();
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
