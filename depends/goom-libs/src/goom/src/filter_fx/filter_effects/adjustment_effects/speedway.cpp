module Goom.FilterFx.FilterEffects.AdjustmentEffects.Speedway;

import Goom.FilterFx.CommonTypes;
import Goom.Utils.NameValuePairs;
import Goom.Utils.Math.GoomRand;

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

using UTILS::NameValuePairs;
using UTILS::MATH::GoomRand;

static constexpr auto AMPLITUDE_RANGE = AmplitudeRange{
    .xRange = {+01.0F, +08.0F},
    .yRange = {-10.0F, +10.0F},
};

static constexpr auto PROB_AMPLITUDE_EQUAL = 0.5F;

Speedway::Speedway(const Modes mode, const GoomRand& goomRand) noexcept
  : m_mode{mode}, m_goomRand{&goomRand}, m_params{GetMode0RandomParams()}
{
}

auto Speedway::SetRandomParams() noexcept -> void
{
  switch (m_mode)
  {
    case Modes::MODE0:
      SetMode0RandomParams();
      break;
    case Modes::MODE1:
      SetMode1RandomParams();
      break;
    case Modes::MODE2:
      SetMode2RandomParams();
      break;
  }
}

auto Speedway::GetMode0RandomParams() const noexcept -> Params
{
  const auto xAmplitude = m_goomRand->GetRandInRange<AMPLITUDE_RANGE.xRange>();
  const auto yAmplitude = m_goomRand->ProbabilityOf<PROB_AMPLITUDE_EQUAL>() ? +1.0F : -1.0F;

  return {xAmplitude, yAmplitude};
}

auto Speedway::GetMode1RandomParams() const noexcept -> Params
{
  const auto xAmplitude = m_goomRand->GetRandInRange<AMPLITUDE_RANGE.xRange>();
  const auto yAmplitude = m_goomRand->ProbabilityOf<PROB_AMPLITUDE_EQUAL>()
                              ? xAmplitude
                              : m_goomRand->GetRandInRange<AMPLITUDE_RANGE.yRange>();

  return {xAmplitude, yAmplitude};
}

auto Speedway::GetMode2RandomParams() const noexcept -> Params
{
  const auto xAmplitude = m_goomRand->GetRandInRange<AMPLITUDE_RANGE.xRange>();
  const auto yAmplitude = m_goomRand->ProbabilityOf<PROB_AMPLITUDE_EQUAL>()
                              ? xAmplitude
                              : m_goomRand->GetRandInRange<AMPLITUDE_RANGE.yRange>();

  return {xAmplitude, yAmplitude};
}

auto Speedway::GetZoomAdjustmentEffectNameValueParams() const noexcept -> NameValuePairs
{
  return {};
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
