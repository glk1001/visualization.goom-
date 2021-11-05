#pragma once

#include <string>

namespace GOOM
{
class PluginInfo;

namespace DRAW
{
  class IGoomDraw;
} // namespace DRAW

namespace UTILS
{
class IGoomRand;
} // namespace UTILS

namespace VISUAL_FX
{

class FxHelpers
{
public:
  FxHelpers(DRAW::IGoomDraw& draw, const PluginInfo& goomInfo, UTILS::IGoomRand& goomRand);

  [[nodiscard]] auto GetDraw() const -> DRAW::IGoomDraw&;
  [[nodiscard]] auto GetDraw() -> DRAW::IGoomDraw&;

  [[nodiscard]] auto GetGoomInfo() const -> const PluginInfo&;

  [[nodiscard]] auto GetGoomRand() const -> UTILS::IGoomRand&;
  [[nodiscard]] auto GetGoomRand() -> UTILS::IGoomRand&;

private:
  DRAW::IGoomDraw& m_draw;
  const PluginInfo& m_goomInfo;
  UTILS::IGoomRand& m_goomRand;
};

inline FxHelpers::FxHelpers(DRAW::IGoomDraw& draw,
                            const PluginInfo& goomInfo,
                            UTILS::IGoomRand& goomRand)
  : m_draw{draw}, m_goomInfo{goomInfo}, m_goomRand{goomRand}
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

inline auto FxHelpers::GetGoomRand() const -> UTILS::IGoomRand&
{
  return m_goomRand;
}

inline auto FxHelpers::GetGoomRand() -> UTILS::IGoomRand&
{
  return m_goomRand;
}

} // namespace VISUAL_FX
} // namespace GOOM
