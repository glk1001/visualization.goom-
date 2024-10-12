module;

#include <string>
#include <vector>

export module Goom.Control.GoomMessageDisplayer;

import Goom.Color.ColorUtils;
import Goom.Draw.GoomDrawBase;
import Goom.Draw.ShaperDrawers.TextDrawer;
import Goom.Lib.GoomGraphic;

using GOOM::COLOR::SimpleColors;
using GOOM::DRAW::IGoomDraw;
using GOOM::DRAW::SHAPE_DRAWERS::TextDrawer;

export namespace GOOM::CONTROL
{

using MessageGroupColors = SimpleColors;
struct MessageGroup
{
  MessageGroupColors color{};
  std::vector<std::string> messages;
};

class GoomMessageDisplayer
{
public:
  GoomMessageDisplayer(IGoomDraw& draw, const std::string& messagesFontFile);

  auto DisplayMessageGroups(const std::vector<MessageGroup>& messageGroups) -> void;

private:
  IGoomDraw* m_draw;

  [[nodiscard]] auto DisplayMessageGroup(int32_t yStart, const MessageGroup& messageGroup)
      -> int32_t;
  // Returns next available y position.

  static constexpr auto MESSAGES_FONT_SIZE = 10;
  Pixel m_currentTextColor;
  TextDrawer m_messagesDisplayer;
  [[nodiscard]] auto GetMessagesDisplayer(const std::string& messagesFontFile) const -> TextDrawer;
};

} // namespace GOOM::CONTROL
