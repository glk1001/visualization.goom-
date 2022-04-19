#include "goom_message_displayer.h"

//#undef NO_LOGGING

#include "color/colorutils.h"
#include "draw/goom_draw.h"
#include "goom/logging.h"
#include "goom_graphic.h"
#include "point2d.h"

#include <vector>

namespace GOOM::CONTROL
{

using COLOR::GetBrighterColor;
using DRAW::IGoomDraw;
using DRAW::TextDraw;
using UTILS::Logging;

GoomMessageDisplayer::GoomMessageDisplayer(IGoomDraw& textOutput,
                                           const std::string& updateMessagesFontFile)
  : m_parallel{textOutput.GetParallel()},
    m_textOutput{textOutput},
    m_updateMessagesFontFile{updateMessagesFontFile},
    m_updateMessagesDisplayers{
        GetUpdateMessagesDisplayers(DEFAULT_NUM_DISPLAYERS, m_textOutput, m_updateMessagesFontFile)}
{
}

auto GoomMessageDisplayer::GetUpdateMessagesDisplayers(const size_t numDisplayers,
                                                       IGoomDraw& textOutput,
                                                       const std::string& updateMessagesFontFile)
    -> std::vector<TextDraw>
{

  const Pixel textColor = GetBrighterColor(10.0F, Pixel::WHITE);
  const auto getFontColor =
      [textColor]([[maybe_unused]] const size_t textIndexOfChar,
                  [[maybe_unused]] const Point2dInt point, [[maybe_unused]] const int32_t width,
                  [[maybe_unused]] const int32_t height) { return textColor; };

  static constexpr uint8_t OUTLINE_GREY = 0xFA;
  const Pixel outlineColor =
      GetBrighterColor(2.0F, Pixel{
                                 {OUTLINE_GREY, OUTLINE_GREY, OUTLINE_GREY, MAX_ALPHA}
  });
  const auto getOutlineFontColor =
      [outlineColor]([[maybe_unused]] const size_t textIndexOfChar,
                     [[maybe_unused]] const Point2dInt point, [[maybe_unused]] const int32_t width,
                     [[maybe_unused]] const int32_t height) { return outlineColor; };

  std::vector<TextDraw> updateMessagesDisplayers{};

  for (size_t i = 0; i < numDisplayers; ++i)
  {
    TextDraw& displayer = updateMessagesDisplayers.emplace_back(textOutput);

    displayer.SetFontFile(updateMessagesFontFile);
    displayer.SetFontSize(MSG_FONT_SIZE);
    displayer.SetOutlineWidth(1);
    displayer.SetAlignment(TextDraw::TextAlignment::LEFT);
    displayer.SetFontColorFunc(getFontColor);
    displayer.SetOutlineFontColorFunc(getOutlineFontColor);
    displayer.SetParallelRender(false);
  }

  return updateMessagesDisplayers;
}

void GoomMessageDisplayer::UpdateMessages(const std::vector<std::string>& msgLines)
{
  if (msgLines.empty())
  {
    return;
  }

  const size_t numberOfLinesInMessage = msgLines.size();
  if (numberOfLinesInMessage > m_updateMessagesDisplayers.size())
  {
    LogInfo("Changing number of displayers from {} to {}.", m_updateMessagesDisplayers.size(),
            numberOfLinesInMessage);
    m_updateMessagesDisplayers =
        GetUpdateMessagesDisplayers(numberOfLinesInMessage, m_textOutput, m_updateMessagesFontFile);
  }

  static constexpr int32_t VERTICAL_SPACING = 10;
  static constexpr size_t LINE_HEIGHT = MSG_FONT_SIZE + VERTICAL_SPACING;
  static constexpr int32_t X_POS = 30;
  static constexpr int32_t Y_START = 10;

  const size_t totalMessagesHeight = 20 + (LINE_HEIGHT * numberOfLinesInMessage);

  const auto displayMessage =
      [this, &totalMessagesHeight, &numberOfLinesInMessage, &msgLines](const size_t i)
  {
    const auto yPos = static_cast<int32_t>((Y_START + totalMessagesHeight) -
                                           ((numberOfLinesInMessage - i) * LINE_HEIGHT));
    m_updateMessagesDisplayers.at(i).SetText(msgLines[i]);
    m_updateMessagesDisplayers.at(i).Prepare();
    m_updateMessagesDisplayers.at(i).Draw({X_POS, yPos});
  };

  m_parallel.ForLoop(numberOfLinesInMessage, displayMessage);
}

} // namespace GOOM::CONTROL
