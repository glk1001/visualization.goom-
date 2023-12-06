#pragma once

#include "blend2d_to_goom.h"
#include "draw/goom_draw.h"
#include "goom/point2d.h"

namespace GOOM::UTILS::GRAPHICS
{

auto FillCircleWithGradient(Blend2dContexts& blend2DContexts,
                            const DRAW::MultiplePixels& colors,
                            float brightness,
                            const Point2dInt& centre,
                            double radius) noexcept -> void;

} // namespace GOOM::UTILS::GRAPHICS
