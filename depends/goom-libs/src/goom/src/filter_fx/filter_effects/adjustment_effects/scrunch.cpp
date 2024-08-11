module Goom.FilterFx.FilterEffects.AdjustmentEffects.Scrunch;

import Goom.FilterFx.CommonTypes;
import Goom.Utils.NameValuePairs;
import Goom.Utils.Math.GoomRand;

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

using UTILS::NameValuePairs;
using UTILS::MATH::GoomRand;

static constexpr auto X_DEFAULT_AMPLITUDE = 0.1F;
static constexpr auto Y_DEFAULT_AMPLITUDE = 1.0F;
static constexpr auto AMPLITUDE_RANGE     = AmplitudeRange{
        .xRange = {0.05F, 0.20F},
        .yRange = {0.50F, 5.00F},
};

Scrunch::Scrunch(const GoomRand& goomRand) noexcept
  : m_goomRand{&goomRand}, m_params{X_DEFAULT_AMPLITUDE, Y_DEFAULT_AMPLITUDE}
{
}

auto Scrunch::SetRandomParams() noexcept -> void
{
  const auto xAmplitude = m_goomRand->GetRandInRange<AMPLITUDE_RANGE.xRange>();
  const auto yAmplitude = m_goomRand->GetRandInRange<AMPLITUDE_RANGE.yRange>();

  SetParams({xAmplitude, yAmplitude});
}

auto Scrunch::GetZoomAdjustmentEffectNameValueParams() const noexcept -> NameValuePairs
{
  return {};
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
