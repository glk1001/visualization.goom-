module;

//#undef NO_LOGGING

#include "goom/goom_logger.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

module Goom.Control.GoomMessageDisplayer;

import Goom.Color.ColorUtils;
import Goom.Draw.GoomDrawBase;
import Goom.Draw.ShaperDrawers.TextDrawer;
import Goom.Lib.GoomGraphic;
import Goom.Lib.GoomTypes;
import Goom.Lib.Point2d;

namespace GOOM::CONTROL
{

using COLOR::GetBrighterColor;
using COLOR::GetSimpleColor;
using DRAW::IGoomDraw;
using DRAW::SHAPE_DRAWERS::TextDrawer;

GoomMessageDisplayer::GoomMessageDisplayer(IGoomDraw& draw, const std::string& messagesFontFile)
  : m_draw{&draw}, m_messagesDisplayer{GetMessagesDisplayer(messagesFontFile)}
{
}

auto GoomMessageDisplayer::GetMessagesDisplayer(const std::string& messagesFontFile) const
    -> TextDrawer
{

  static constexpr auto TEXT_BRIGHTNESS = 50.0F;
  const auto getFontColor               = [this]([[maybe_unused]] const size_t textIndexOfChar,
                                   [[maybe_unused]] const Point2dInt& point,
                                   [[maybe_unused]] const Dimensions& charDimensions)
  { return GetBrighterColor(TEXT_BRIGHTNESS, m_currentTextColor); };

  static constexpr auto OUTLINE_BRIGHTNESS = TEXT_BRIGHTNESS;
  // const auto outlineColor                  = GetBrighterColor(OUTLINE_BRIGHTNESS, WHITE_PIXEL);
  const auto getOutlineFontColor = [this]([[maybe_unused]] const size_t textIndexOfChar,
                                          [[maybe_unused]] const Point2dInt& point,
                                          [[maybe_unused]] const Dimensions& charDimensions)
  { return GetBrighterColor(OUTLINE_BRIGHTNESS, m_currentTextColor); };

  auto displayer = TextDrawer{*m_draw};

  displayer.SetFontFile(messagesFontFile);
  displayer.SetFontSize(MESSAGES_FONT_SIZE);
  displayer.SetOutlineWidth(1);
  displayer.SetAlignment(TextDrawer::TextAlignment::LEFT);
  displayer.SetFontColorFunc(getFontColor);
  displayer.SetOutlineFontColorFunc(getOutlineFontColor);
  displayer.SetParallelRender(false);

  return displayer;
}

auto GoomMessageDisplayer::DisplayMessageGroups(const std::vector<MessageGroup>& messageGroups)
    -> void
{
  static constexpr auto Y_START = 0;

  auto y = Y_START;
  for (const auto& messageGroup : messageGroups)
  {
    m_currentTextColor = GetSimpleColor(messageGroup.color, MAX_ALPHA);
    y                  = DisplayMessageGroup(y, messageGroup);
  }
}

auto GoomMessageDisplayer::DisplayMessageGroup(const int32_t yStart,
                                               const MessageGroup& messageGroup) -> int32_t
{
  if (messageGroup.messages.empty())
  {
    return yStart;
  }

  const auto numberOfLinesInMessage = messageGroup.messages.size();

  static constexpr auto VERTICAL_SPACING      = 10;
  static constexpr auto LINE_HEIGHT           = MESSAGES_FONT_SIZE + VERTICAL_SPACING;
  static constexpr auto X_POS                 = 30;
  static constexpr auto BETWEEN_GROUP_SPACING = LINE_HEIGHT / 2;

  const auto totalMessagesHeight = 20U + (LINE_HEIGHT * numberOfLinesInMessage);

  auto yPos = 0;
  for (auto i = 0U; i < numberOfLinesInMessage; ++i)
  {
    yPos = yStart +
           static_cast<int32_t>(totalMessagesHeight - ((numberOfLinesInMessage - i) * LINE_HEIGHT));

    m_messagesDisplayer.SetText(messageGroup.messages[i]);
    m_messagesDisplayer.Prepare();
    m_messagesDisplayer.Draw({.x = X_POS, .y = yPos});
  }

  return yPos + BETWEEN_GROUP_SPACING;
}

} // namespace GOOM::CONTROL
