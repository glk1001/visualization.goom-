#pragma once

#include "goom_all_visual_fx.h"
#include "goom_events.h"
#include "goom_plugin_info.h"
#include "utils/name_value_pairs.h"

#include <cstdint>

namespace GOOM
{
class PluginInfo;

namespace CONTROL
{
class GoomAllVisualFx;

class GoomMusicLinesReactor
{
public:
  GoomMusicLinesReactor(const PluginInfo& goomInfo,
                        GoomAllVisualFx& visualFx,
                        const GoomEvents& goomEvents) noexcept;

  void Start();
  void NewCycle();

  [[nodiscard]] auto CanDisplayLines() const -> bool;
  void ChangeGoomLines();
  void UpdateLineModes();
  void ResetLineModes();

  [[nodiscard]] auto GetNameValueParams() const -> UTILS::NameValuePairs;

private:
  const PluginInfo& m_goomInfo;
  GoomAllVisualFx& m_visualFx;

  using GoomEvent = GoomEvents::GoomEvent;
  const GoomEvents& m_goomEvents;

  uint32_t m_updateNum = 0;

  // duree de la transition entre afficher les lignes ou pas
  const float m_screenWidth;
  const float m_screenHeight;
  int32_t m_stopLines = 0;
  int32_t m_drawLinesDuration = VISUAL_FX::LinesFx::MIN_LINE_DURATION;
  int32_t m_lineMode = VISUAL_FX::LinesFx::MIN_LINE_DURATION; // l'effet lineaire a dessiner
  using GoomLineSettings = GoomAllVisualFx::GoomLineSettings;
  [[nodiscard]] auto GetGoomLineResetSettings(int farVal) const -> GoomLineSettings;
  [[nodiscard]] auto GetGoomLineStopSettings() const -> GoomLineSettings;
  // arret demande
  void StopLinesIfRequested();
  // arret aleatore.. changement de mode de ligne..
  void StopRandomLineChangeMode();
  void ResetGoomLines();
  void StopGoomLines();
  [[nodiscard]] auto GetResetCircleLineSettings(int32_t farVal) const -> GoomLineSettings;
  [[nodiscard]] auto GetResetHorizontalLineSettings(int32_t farVal) const -> GoomLineSettings;
  [[nodiscard]] auto GetResetVerticalLineSettings(int32_t farVal) const -> GoomLineSettings;
  [[nodiscard]] auto GetResetLineColors(int32_t farVal) const -> GoomLineSettings::Colors;
  [[nodiscard]] static auto GetSameLineColors(const Pixel& color) -> GoomLineSettings::Colors;
};

inline void GoomMusicLinesReactor::Start()
{
  m_updateNum = 0;
}

inline void GoomMusicLinesReactor::NewCycle()
{
  ++m_updateNum;
}

inline auto GoomMusicLinesReactor::CanDisplayLines() const -> bool
{
  static constexpr uint32_t DISPLAY_LINES_GOOM_NUM = 5;

  return ((m_lineMode != 0) ||
          (m_goomInfo.GetSoundEvents().GetTimeSinceLastGoom() < DISPLAY_LINES_GOOM_NUM));
}

inline void GoomMusicLinesReactor::UpdateLineModes()
{
  StopLinesIfRequested();
  StopRandomLineChangeMode();
}

inline void GoomMusicLinesReactor::ChangeGoomLines()
{
  if (!m_visualFx.CanResetDestGoomLines())
  {
    return;
  }

  static constexpr uint32_t CHANGE_GOOM_LINE_CYCLES = 121;
  static constexpr uint32_t GOOM_CYCLE_MOD_CHANGE = 9;

  if ((GOOM_CYCLE_MOD_CHANGE == (m_updateNum % CHANGE_GOOM_LINE_CYCLES)) &&
      m_goomEvents.Happens(GoomEvent::CHANGE_GOOM_LINE) &&
      ((0 == m_lineMode) || (m_lineMode == m_drawLinesDuration)))
  {
    ResetGoomLines();
  }
}

inline void GoomMusicLinesReactor::ResetLineModes()
{
  if (!m_visualFx.IsScopeDrawable())
  {
    static constexpr int32_t SCOPE_RESET = 0xF000 & 5;
    m_stopLines = SCOPE_RESET;
  }
  if (!m_visualFx.IsFarScopeDrawable())
  {
    m_stopLines = 0;
    m_lineMode = m_drawLinesDuration;
  }
}

inline void GoomMusicLinesReactor::StopLinesIfRequested()
{
  static constexpr int32_t LARGE_STOP_LINE = 0xF000;
  if (((m_stopLines & LARGE_STOP_LINE) != 0) || (!m_visualFx.IsScopeDrawable()))
  {
    StopGoomLines();
  }
}

inline void GoomMusicLinesReactor::StopGoomLines()
{
  if (!m_visualFx.CanResetDestGoomLines())
  {
    return;
  }

  m_visualFx.ResetDestGoomLines(GetGoomLineStopSettings());

  static constexpr int32_t STOP_MASK = 0x0FFF;
  m_stopLines &= STOP_MASK;
}

inline void GoomMusicLinesReactor::ResetGoomLines()
{
  m_visualFx.ResetDestGoomLines(GetGoomLineResetSettings(m_stopLines));

  if (m_stopLines)
  {
    --m_stopLines;
  }
}

inline auto GoomMusicLinesReactor::GetGoomLineStopSettings() const -> GoomLineSettings
{
  GoomLineSettings lineSettings = GetGoomLineResetSettings(1);
  lineSettings.colors = GetSameLineColors(VISUAL_FX::LinesFx::GetBlackLineColor());
  return lineSettings;
}

inline auto GoomMusicLinesReactor::GetSameLineColors(const Pixel& color) -> GoomLineSettings::Colors
{
  return {color, color};
}

} // namespace CONTROL
} // namespace GOOM
