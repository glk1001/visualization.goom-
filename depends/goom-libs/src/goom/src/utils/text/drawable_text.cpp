//#undef NO_LOGGING

#include "drawable_text.h"

#include "draw/shape_drawers/text_drawer.h"
#include "goom/goom_logger.h"
#include "goom/point2d.h"

#include <cstdint>
#include <string>
#include <vector>

import Goom.Utils.StrUtils;
import Goom.Utils.Math.Misc;

namespace GOOM::UTILS::TEXT
{

using namespace std::string_literals;
using DRAW::SHAPE_DRAWERS::TextDrawer;
using MATH::I_HALF;

auto GetLinesOfWords(const std::string& text, const uint32_t maxLineLength)
    -> std::vector<std::string>
{
  if (text.length() <= maxLineLength)
  {
    return {text};
  }

  const auto words = StringSplit(text, " ");
  auto textLines   = std::vector<std::string>{};
  auto str         = ""s; // NOLINT: Supposed to be in 'string' header.

  for (const auto& word : words)
  {
    str += word + " ";
    if (str.length() > maxLineLength)
    {
      textLines.emplace_back(str);
      str = "";
    }
  }

  if (!str.empty())
  {
    textLines.emplace_back(str);
  }

  return textLines;
}

auto GetLeftAlignedPenForCentringStringAt(TextDrawer& textDrawer,
                                          const std::string& text,
                                          const int32_t fontSize,
                                          const Point2dInt& centreAt) -> Point2dInt
{
  const auto oldFontSize    = textDrawer.GetFontSize();
  const auto oldCharSpacing = textDrawer.GetCharSpacing();

  textDrawer.SetFontSize(fontSize);
  textDrawer.SetCharSpacing(0.0F);
  textDrawer.SetText(text);

  textDrawer.Prepare();

  const auto strRect    = textDrawer.GetPreparedTextBoundingRect();
  const auto bearingX   = textDrawer.GetBearingX();
  const auto bearingY   = textDrawer.GetBearingY();
  const auto textWidth  = (strRect.xMax - strRect.xMin) + 1;
  const auto textHeight = (strRect.yMax - strRect.yMin) + 1;

  LogInfo("font size = {}", textDrawer.GetFontSize());
  LogInfo("charSpacing = {}", textDrawer.GetCharSpacing());
  LogInfo("alignment = {}", textDrawer.GetAlignment());
  LogInfo("text = {}", text);
  LogInfo("textWidth = {}, textHeight = {}", textWidth, textHeight);
  LogInfo("bearingX = {}, bearingY = {}", bearingX, bearingY);
  LogInfo("centre pen = ({},{})",
          centreAt.x - (I_HALF * (textWidth - bearingX)),
          centreAt.y - ((I_HALF * textHeight) - bearingY));

  textDrawer.SetCharSpacing(oldCharSpacing);
  textDrawer.SetFontSize(oldFontSize);

  return {centreAt.x - (I_HALF * (textWidth - bearingX)),
          centreAt.y - ((I_HALF * textHeight) - bearingY)};
}

} // namespace GOOM::UTILS::TEXT
