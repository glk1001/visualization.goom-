#pragma once

#include <string>

namespace GOOM
{
class PluginInfo;

namespace DRAW
{
  class IGoomDraw;
} // namespace DRAW

namespace VISUAL_FX
{

class FxHelpers
{
public:
  FxHelpers(DRAW::IGoomDraw& draw, const PluginInfo& goomInfo);

  [[nodiscard]] auto GetDraw() const -> DRAW::IGoomDraw&;
  [[nodiscard]] auto GetDraw() -> DRAW::IGoomDraw&;

  [[nodiscard]] auto GetGoomInfo() const -> const PluginInfo&;
private:
  DRAW::IGoomDraw& m_draw;
  const PluginInfo& m_goomInfo;
};

inline FxHelpers::FxHelpers(DRAW::IGoomDraw& draw, const PluginInfo& goomInfo)
: m_draw{draw}, m_goomInfo{goomInfo}
{
}

inline auto FxHelpers::GetDraw() const -> DRAW::IGoomDraw&
{
  return m_draw;
}

inline auto FxHelpers::GetDraw() -> DRAW::IGoomDraw&
{
  return m_draw;
}

inline auto FxHelpers::GetGoomInfo() const -> const PluginInfo&
{
  return m_goomInfo;
}

} // namespace VISUAL_FX
} // namespace GOOM
