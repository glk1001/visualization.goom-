#ifndef VISUALIZATION_GOOM_GOOM_MESSAGE_DISPLAYER_H
#define VISUALIZATION_GOOM_GOOM_MESSAGE_DISPLAYER_H

#include "../draw/text_draw.h"

#include <string>

namespace GOOM
{
class IGoomDraw;

namespace CONTROL
{

class GoomMessageDisplayer
{
public:
  GoomMessageDisplayer(IGoomDraw& textOutput, const std::string& updateMessagesFontFile);

  void UpdateMessages(const std::string& messages);

private:
  static constexpr int32_t MSG_FONT_SIZE = 10;
  DRAW::TextDraw m_updateMessagesDisplay;
};

} // namespace CONTROL
} // namespace GOOM

#endif //VISUALIZATION_GOOM_GOOM_MESSAGE_DISPLAYER_H
