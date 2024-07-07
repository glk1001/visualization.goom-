export module Goom.Utils.Graphics.TestPatterns;

import Goom.Draw.GoomDrawBase;
import Goom.Lib.GoomTypes;
import Goom.Lib.Point2d;

export namespace GOOM::UTILS::GRAPHICS
{

auto DrawTestPattern(DRAW::IGoomDraw& draw,
                     const Point2dInt& centre,
                     const Dimensions& dimensions) -> void;

} // namespace GOOM::UTILS::GRAPHICS
