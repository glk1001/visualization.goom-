#pragma once

#include "draw/goom_draw.h"
#include "draw/shape_drawers/line_drawer.h"
#include "goom_types.h"
#include "point2d.h"

namespace GOOM::UTILS::GRAPHICS
{

auto DrawTestPattern(DRAW::IGoomDraw& draw, const Dimensions& dimensions) -> void;

} // namespace GOOM::UTILS::GRAPHICS
