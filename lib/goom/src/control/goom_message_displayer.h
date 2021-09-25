#pragma once

#include "draw/text_draw.h"

#include <string>

namespace GOOM
{

namespace DRAW
{
class IGoomDraw;
} // namespace DRAW

namespace CONTROL
{

class GoomMessageDisplayer
{
public:
  GoomMessageDisplayer(DRAW::IGoomDraw& textOutput, const std::string& updateMessagesFontFile);

  void UpdateMessages(const std::string& messages);

private:
  static constexpr int32_t MSG_FONT_SIZE = 10;
  DRAW::TextDraw m_updateMessagesDisplay;
};

} // namespace CONTROL
} // namespace GOOM

