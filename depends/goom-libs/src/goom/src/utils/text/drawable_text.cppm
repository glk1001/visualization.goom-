module;

#include "goom/goom_config.h"

#include <cstdint>
#include <string>
#include <vector>

export module Goom.Utils.Text.DrawableText;

import Goom.Draw.ShaperDrawers.TextDrawer;
import Goom.Lib.Point2d;

export namespace GOOM::UTILS::TEXT
{

[[nodiscard]] auto GetLinesOfWords(const std::string& text, uint32_t maxLineLength)
    -> std::vector<std::string>;

[[nodiscard]] auto GetLeftAlignedPenForCentringStringAt(DRAW::SHAPE_DRAWERS::TextDrawer& textDrawer,
                                                        const std::string& text,
                                                        int32_t fontSize,
                                                        const Point2dInt& centreAt) -> Point2dInt;

} // namespace GOOM::UTILS::TEXT
