module;

#include "goom/goom_config.h"

module Goom.FilterFx.FilterEffects.AdjustmentEffects.Amulet;

import Goom.FilterFx.FilterUtils.Utils;
import Goom.FilterFx.NormalizedCoords;
import Goom.Utils.NameValuePairs;
import Goom.Utils.Math.GoomRandBase;

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

using FILTER_UTILS::RandomViewport;
using UTILS::NameValuePairs;
using UTILS::MATH::IGoomRand;

static constexpr auto DEFAULT_AMPLITUDE = 1.0F;
static constexpr auto AMPLITUDE_RANGE   = IGoomRand::NumberRange<float>{0.1F, 1.51F};

static constexpr auto VIEWPORT_BOUNDS = RandomViewport::Bounds{
    .minSideLength       = 0.1F,
    .probUseCentredSides = 1.0F,
    .rect                = {},
    .sides               = {.minMaxWidth = {2.0F, 10.0F}, .minMaxHeight = {2.0F, 10.0F}}
};

static constexpr auto PROB_XY_AMPLITUDES_EQUAL = 0.98F;
static constexpr auto PROB_NO_VIEWPORT         = 0.5F;

Amulet::Amulet(const IGoomRand& goomRand) noexcept
  : m_goomRand{&goomRand},
    m_randomViewport{goomRand, VIEWPORT_BOUNDS},
    m_params{Viewport{}, {DEFAULT_AMPLITUDE, DEFAULT_AMPLITUDE}}
{
  m_randomViewport.SetProbNoViewport(PROB_NO_VIEWPORT);
}

auto Amulet::SetRandomParams() noexcept -> void
{
  const auto viewport = m_randomViewport.GetRandomViewport();

  const auto xAmplitude = m_goomRand->GetRandInRange(AMPLITUDE_RANGE);
  const auto yAmplitude = m_goomRand->ProbabilityOf(PROB_XY_AMPLITUDES_EQUAL)
                              ? xAmplitude
                              : m_goomRand->GetRandInRange(AMPLITUDE_RANGE);

  SetParams({
      viewport, {xAmplitude, yAmplitude}
  });
}

auto Amulet::GetZoomAdjustmentEffectNameValueParams() const noexcept -> NameValuePairs
{
  return NameValuePairs();
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
