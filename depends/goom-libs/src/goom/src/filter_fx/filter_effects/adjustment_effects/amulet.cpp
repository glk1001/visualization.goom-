module Goom.FilterFx.FilterEffects.AdjustmentEffects.Amulet;

import Goom.FilterFx.FilterUtils.Utils;
import Goom.FilterFx.NormalizedCoords;
import Goom.Utils.NameValuePairs;
import Goom.Utils.Math.GoomRand;

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

using FILTER_UTILS::RandomViewport;
using UTILS::NameValuePairs;
using UTILS::MATH::GoomRand;
using UTILS::MATH::NumberRange;

static constexpr auto AMPLITUDE_RANGE = NumberRange{0.1F, 1.51F};

static constexpr auto VIEWPORT_BOUNDS = RandomViewport::Bounds{
    .minSideLength       = 0.1F,
    .probUseCentredSides = 1.0F,
    .rect                = {},
    .sides               = {.minMaxWidth  = {.minValue = 2.0F, .maxValue = 10.0F},
                            .minMaxHeight = {.minValue = 2.0F, .maxValue = 10.0F}}
};

static constexpr auto PROB_XY_AMPLITUDES_EQUAL = 0.98F;
static constexpr auto PROB_NO_VIEWPORT         = 0.5F;

Amulet::Amulet(const GoomRand& goomRand) noexcept
  : m_goomRand{&goomRand}, m_randomViewport{goomRand, VIEWPORT_BOUNDS}, m_params{GetRandomParams()}
{
  m_randomViewport.SetProbNoViewport(PROB_NO_VIEWPORT);
}

auto Amulet::GetRandomParams() const noexcept -> Params
{
  const auto viewport = m_randomViewport.GetRandomViewport();

  const auto xAmplitude = m_goomRand->GetRandInRange<AMPLITUDE_RANGE>();
  const auto yAmplitude = m_goomRand->ProbabilityOf<PROB_XY_AMPLITUDES_EQUAL>()
                              ? xAmplitude
                              : m_goomRand->GetRandInRange<AMPLITUDE_RANGE>();

  return {
      .viewport = viewport, .amplitude = {xAmplitude, yAmplitude}
  };
}

auto Amulet::GetZoomAdjustmentEffectNameValueParams() const noexcept -> NameValuePairs
{
  return {};
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
