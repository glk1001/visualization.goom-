module;

#include "draw/goom_draw.h"
#include "goom/point2d.h"

#include <blend2d.h> // NOLINT(misc-include-cleaner): Blend2d insists on this.
#include <blend2d/context.h>

export module Goom.Utils.Graphics.Blend2dUtils;

import Goom.Utils.Graphics.Blend2dToGoom;

export namespace GOOM::UTILS::GRAPHICS
{

auto FillCircleWithGradient(Blend2dContexts& blend2DContexts,
                            const DRAW::MultiplePixels& colors,
                            float brightness,
                            const Point2dInt& centre,
                            double radius) noexcept -> void;

} // namespace GOOM::UTILS::GRAPHICS
