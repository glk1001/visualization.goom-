#pragma once

#include "../goom_draw.h"
#include "goom_config.h"
#include "point2d.h"

#include <functional>

namespace GOOM::DRAW::SHAPE_DRAWERS
{

using PixelDrawerFunc = std::function<void(const Point2dInt& point, const MultiplePixels& colors)>;

} // namespace GOOM::DRAW::SHAPE_DRAWERS
