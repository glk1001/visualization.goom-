module;

#include "goom/point2d.h"

export module Goom.Utils.Graphics.TestPatterns;

import Goom.Draw.GoomDrawBase;
import Goom.Lib.GoomTypes;

export namespace GOOM::UTILS::GRAPHICS
{

auto DrawTestPattern(DRAW::IGoomDraw& draw, const Point2dInt& centre, const Dimensions& dimensions)
    -> void;

} // namespace GOOM::UTILS::GRAPHICS
