module;

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

export module Goom.Control.GoomMessageDisplayer;

import Goom.Draw.GoomDrawBase;
import Goom.Draw.ShaperDrawers.TextDrawer;
import Goom.Utils.Parallel;

export namespace GOOM::CONTROL
{

class GoomMessageDisplayer
{
public:
  GoomMessageDisplayer(DRAW::IGoomDraw& draw, const std::string& updateMessagesFontFile);

  void UpdateMessages(const std::vector<std::string>& msgLines);

private:
  UTILS::Parallel m_parallel{UTILS::GetNumAvailablePoolThreads()};
  DRAW::IGoomDraw* m_draw;
  std::string m_updateMessagesFontFile;

  static constexpr int32_t MSG_FONT_SIZE         = 9;
  static constexpr size_t DEFAULT_NUM_DISPLAYERS = 1;
  std::vector<DRAW::SHAPE_DRAWERS::TextDrawer> m_updateMessagesDisplayers{
      GetUpdateMessagesDisplayers(DEFAULT_NUM_DISPLAYERS, *m_draw, m_updateMessagesFontFile)};
  [[nodiscard]] static auto GetUpdateMessagesDisplayers(size_t numDisplayers,
                                                        DRAW::IGoomDraw& textOutput,
                                                        const std::string& updateMessagesFontFile)
      -> std::vector<DRAW::SHAPE_DRAWERS::TextDrawer>;
};

} // namespace GOOM::CONTROL
