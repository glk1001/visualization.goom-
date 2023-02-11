#pragma once

#include <string>

namespace GOOM
{
class GoomLogger;
class PluginInfo;

namespace DRAW
{
class IGoomDraw;
}

namespace UTILS::MATH
{
class IGoomRand;
}

namespace VISUAL_FX
{

class FxHelper
{
public:
  FxHelper(DRAW::IGoomDraw& draw,
           const PluginInfo& goomInfo,
           const UTILS::MATH::IGoomRand& goomRand,
           GoomLogger& goomLogger);

  [[nodiscard]] auto GetDraw() const -> DRAW::IGoomDraw&;
  [[nodiscard]] auto GetDraw() -> DRAW::IGoomDraw&;
  [[nodiscard]] auto GetGoomInfo() const -> const PluginInfo&;
  [[nodiscard]] auto GetGoomRand() const -> const UTILS::MATH::IGoomRand&;
  [[nodiscard]] auto GetGoomLogger() const -> GoomLogger&;

private:
  DRAW::IGoomDraw* m_draw;
  const PluginInfo* m_goomInfo;
  const UTILS::MATH::IGoomRand* m_goomRand;
  GoomLogger* m_goomLogger;
};

inline FxHelper::FxHelper(DRAW::IGoomDraw& draw,
                          const PluginInfo& goomInfo,
                          const UTILS::MATH::IGoomRand& goomRand,
                          GoomLogger& goomLogger)
  : m_draw{&draw}, m_goomInfo{&goomInfo}, m_goomRand{&goomRand}, m_goomLogger{&goomLogger}
{
}

inline auto FxHelper::GetDraw() const -> DRAW::IGoomDraw&
{
  return *m_draw;
}

inline auto FxHelper::GetDraw() -> DRAW::IGoomDraw&
{
  return *m_draw;
}

inline auto FxHelper::GetGoomInfo() const -> const PluginInfo&
{
  return *m_goomInfo;
}

inline auto FxHelper::GetGoomRand() const -> const UTILS::MATH::IGoomRand&
{
  return *m_goomRand;
}

inline auto FxHelper::GetGoomLogger() const -> GoomLogger&
{
  return *m_goomLogger;
}

} // namespace VISUAL_FX
} // namespace GOOM
