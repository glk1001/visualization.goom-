module;

#include "draw/goom_draw.h"
#include "goom/goom_types.h"
#include "goom/point2d.h"

export module Goom.Utils.Graphics.TestPatterns;

export namespace GOOM::UTILS::GRAPHICS
{

auto DrawTestPattern(DRAW::IGoomDraw& draw, const Point2dInt& centre, const Dimensions& dimensions)
    -> void;

} // namespace GOOM::UTILS::GRAPHICS
