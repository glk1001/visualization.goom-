#include "goom_message_displayer.h"

#include "draw/goom_draw.h"
#include "goom_graphic.h"
#include "utils/strutils.h"

#include <vector>

namespace GOOM::CONTROL
{

using DRAW::TextDraw;
using DRAW::IGoomDraw;
using UTILS::StringSplit;

GoomMessageDisplayer::GoomMessageDisplayer(IGoomDraw& textOutput,
                                           const std::string& updateMessagesFontFile)
  : m_updateMessagesDisplay{textOutput}
{
  const auto getFontColor = []([[maybe_unused]] const size_t textIndexOfChar,
                               [[maybe_unused]] const int32_t x, [[maybe_unused]] const int32_t y,
                               [[maybe_unused]] const int32_t width,
                               [[maybe_unused]] const int32_t height) { return Pixel::WHITE; };
  const auto getOutlineFontColor =
      []([[maybe_unused]] const size_t textIndexOfChar, [[maybe_unused]] const int32_t x,
         [[maybe_unused]] const int32_t y, [[maybe_unused]] const int32_t width,
         [[maybe_unused]] const int32_t height)
  {
    constexpr uint8_t OUTLINE_GREY = 0xFA;
    return Pixel{
        {OUTLINE_GREY, OUTLINE_GREY, OUTLINE_GREY, MAX_ALPHA}
    };
  };
  m_updateMessagesDisplay.SetFontFile(updateMessagesFontFile);
  m_updateMessagesDisplay.SetFontSize(MSG_FONT_SIZE);
  m_updateMessagesDisplay.SetOutlineWidth(1);
  m_updateMessagesDisplay.SetAlignment(TextDraw::TextAlignment::LEFT);
  m_updateMessagesDisplay.SetFontColorFunc(getFontColor);
  m_updateMessagesDisplay.SetOutlineFontColorFunc(getOutlineFontColor);
}

void GoomMessageDisplayer::UpdateMessages(const std::string& messages)
{
  if (messages.empty())
  {
    return;
  }

  constexpr int32_t VERTICAL_SPACING = 10;
  constexpr size_t LINE_HEIGHT = MSG_FONT_SIZE + VERTICAL_SPACING;
  constexpr int32_t X_POS = 50;
  constexpr int32_t Y_START = 50;

  const std::vector<std::string> msgLines = StringSplit(messages, "\n");
  const size_t numberOfLinesInMessage = msgLines.size();
  const size_t totalMessagesHeight = 20 + (LINE_HEIGHT * numberOfLinesInMessage);

  for (size_t i = 0; i < msgLines.size(); ++i)
  {
    const auto yPos = static_cast<int32_t>((Y_START + totalMessagesHeight) -
                                           ((numberOfLinesInMessage - i) * LINE_HEIGHT));
    m_updateMessagesDisplay.SetText(msgLines[i]);
    m_updateMessagesDisplay.Prepare();
    m_updateMessagesDisplay.Draw(X_POS, yPos);
  }
}

} // namespace GOOM::CONTROL
