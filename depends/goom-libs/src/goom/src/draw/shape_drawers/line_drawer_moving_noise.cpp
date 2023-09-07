#include "line_drawer_moving_noise.h"

#include "draw/goom_draw.h"
#include "goom/goom_config.h"
#include "goom/goom_types.h"
#include "utils/math/goom_rand_base.h"
#include "utils/t_values.h"

#include <cstdint>

namespace GOOM::DRAW::SHAPE_DRAWERS
{

using UTILS::TValue;
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
