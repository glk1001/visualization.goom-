#pragma once

#include <string>

namespace GOOM
{
class PluginInfo;

namespace DRAW
{
class IGoomDraw;
}

namespace UTILS
{
class IGoomRand;
}

namespace VISUAL_FX
{

class FxHelper
{
public:
  FxHelper(DRAW::IGoomDraw& draw, const PluginInfo& goomInfo, const UTILS::IGoomRand& goomRand);

  [[nodiscard]] auto GetDraw() const -> DRAW::IGoomDraw&;
  [[nodiscard]] auto GetDraw() -> DRAW::IGoomDraw&;

  [[nodiscard]] auto GetGoomInfo() const -> const PluginInfo&;

  [[nodiscard]] auto GetGoomRand() const -> const UTILS::IGoomRand&;

private:
  DRAW::IGoomDraw& m_draw;
  const PluginInfo& m_goomInfo;
  const UTILS::IGoomRand& m_goomRand;
};

inline FxHelper::FxHelper(DRAW::IGoomDraw& draw,
                          const PluginInfo& goomInfo,
                          const UTILS::IGoomRand& goomRand)
  : m_draw{draw}, m_goomInfo{goomInfo}, m_goomRand{goomRand}
{
}

inline auto FxHelper::GetDraw() const -> DRAW::IGoomDraw&
{
  return m_draw;
}

inline auto FxHelper::GetDraw() -> DRAW::IGoomDraw&
{
  return m_draw;
}

inline auto FxHelper::GetGoomInfo() const -> const PluginInfo&
{
  return m_goomInfo;
}

inline auto FxHelper::GetGoomRand() const -> const UTILS::IGoomRand&
{
  return m_goomRand;
}

} // namespace VISUAL_FX
} // namespace GOOM
