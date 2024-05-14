module;

#include "goom/goom_config.h"
#include "goom/goom_types.h"

#include <cstdint>

module Goom.Draw.ShaperDrawers.LineDrawerMovingNoise;

import Goom.Draw.GoomDrawBase;
import Goom.Utils.Math.TValues;
import Goom.Utils.Math.Misc;
import Goom.Utils.Math.GoomRandBase;

namespace GOOM::DRAW::SHAPE_DRAWERS
{

using UTILS::MATH::TValue;
using UTILS::MATH::IGoomRand;

LineDrawerMovingNoise::LineDrawerMovingNoise(IGoomDraw& draw,
                                             const IGoomRand& goomRand,
                                             const MinMaxValues<uint8_t>& minMaxNoiseRadius,
                                             const uint32_t numNoiseRadiusSteps,
                                             const MinMaxValues<uint8_t>& minMaxNumNoisePixels,
                                             const uint32_t numNumPixelSteps) noexcept
  : m_lineDrawer{draw, goomRand, {minMaxNoiseRadius.minValue, minMaxNumNoisePixels.minValue}},
    m_noiseRadius{minMaxNoiseRadius.minValue,
                  minMaxNoiseRadius.maxValue,
                  TValue::StepType::CONTINUOUS_REVERSIBLE,
                  numNoiseRadiusSteps},
    m_numNoisePixelsPerPixel{minMaxNumNoisePixels.minValue,
                             minMaxNumNoisePixels.maxValue,
                             TValue::StepType::CONTINUOUS_REVERSIBLE,
                             numNumPixelSteps}
{
}

} // namespace GOOM::DRAW::SHAPE_DRAWERS
