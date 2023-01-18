#pragma once

#include "point2d.h"

#include <functional>

namespace GOOM::DRAW::SHAPE_DRAWERS::WU_LINES
{

// The Xiaolin Wu anti-aliased line draw.
// From https://rosettacode.org/wiki/Xiaolin_Wu%27s_line_algorithm#C.2B.2B
//

using PlotPointFunc = const std::function<void(const Point2dInt& point, float brightness)>;

auto WuLine(float x0, float y0, float x1, float y1, const PlotPointFunc& plot) noexcept -> void;

} // namespace GOOM::DRAW::SHAPE_DRAWERS::WU_LINES
