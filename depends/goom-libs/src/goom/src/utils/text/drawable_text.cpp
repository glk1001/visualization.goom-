#include "drawable_text.h"

//#undef NO_LOGGING

#include "draw/text_draw.h"
#include "goom/logging.h"
#include "point2d.h"
#include "utils/strutils.h"

#include <string>
#include <vector>

namespace GOOM::UTILS::TEXT
{

using DRAW::TextDraw;
using MATH::I_HALF;

auto GetLinesOfWords(const std::string& text, const uint32_t maxLineLength)
    -> std::vector<std::string>
{
  if (text.length() <= maxLineLength)
  {
    return {text};
  }

  const std::vector<std::string> words = StringSplit(text, " ");
  std::vector<std::string> textLines{};
  std::string str{};

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

auto GetLeftAlignedPenForCentringStringAt(TextDraw& textDraw,
                                          const std::string& text,
                                          const int32_t fontSize,
                                          const Point2dInt& centreAt) -> Point2dInt
{
  const int32_t oldFontSize = textDraw.GetFontSize();
  const float oldCharSpacing = textDraw.GetCharSpacing();

  textDraw.SetFontSize(fontSize);
  textDraw.SetCharSpacing(0.0F);
  textDraw.SetText(text);

  textDraw.Prepare();

  const TextDraw::Rect strRect = textDraw.GetPreparedTextBoundingRect();
  const int32_t bearingX = textDraw.GetBearingX();
  const int32_t bearingY = textDraw.GetBearingY();
  const int32_t textWidth = (strRect.xMax - strRect.xMin) + 1;
  const int32_t textHeight = (strRect.yMax - strRect.yMin) + 1;

  LogInfo("font size = {}", textDraw.GetFontSize());
  LogInfo("charSpacing = {}", textDraw.GetCharSpacing());
  LogInfo("alignment = {}", textDraw.GetAlignment());
  LogInfo("text = {}", text);
  LogInfo("textWidth = {}, textHeight = {}", textWidth, textHeight);
  LogInfo("bearingX = {}, bearingY = {}", bearingX, bearingY);
  LogInfo("centre pen = ({},{})", centreAt.x - (I_HALF * (textWidth - bearingX)),
          centreAt.y - ((I_HALF * textHeight) - bearingY));

  textDraw.SetCharSpacing(oldCharSpacing);
  textDraw.SetFontSize(oldFontSize);

  return {centreAt.x - (I_HALF * (textWidth - bearingX)),
          centreAt.y - ((I_HALF * textHeight) - bearingY)};
}

} // namespace GOOM::UTILS::TEXT
