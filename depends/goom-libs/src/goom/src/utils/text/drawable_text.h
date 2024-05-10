#pragma once

#include "goom/goom_config.h"

#include <cstdint>
#include <string>
#include <vector>

import Goom.Draw.ShaperDrawers.TextDrawer;
import Goom.Lib.Point2d;

namespace GOOM::UTILS::TEXT
{

[[nodiscard]] auto GetLinesOfWords(const std::string& text, uint32_t maxLineLength)
    -> std::vector<std::string>;

[[nodiscard]] auto GetLeftAlignedPenForCentringStringAt(DRAW::SHAPE_DRAWERS::TextDrawer& textDrawer,
                                                        const std::string& text,
                                                        int32_t fontSize,
                                                        const Point2dInt& centreAt) -> Point2dInt;

} // namespace GOOM::UTILS::TEXT
