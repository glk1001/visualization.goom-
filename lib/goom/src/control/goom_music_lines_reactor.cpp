#include "goom_music_lines_reactor.h"

#include "goom_events.h"
#include "goomutils/name_value_pairs.h"

#if __cplusplus <= 201402L
namespace GOOM
{
namespace CONTROL
{
#else
namespace GOOM::CONTROL
{
#endif

using UTILS::GetPair;
using UTILS::NameValuePairs;

GoomMusicLinesReactor::GoomMusicLinesReactor(const PluginInfo& goomInfo,
                                             GoomAllVisualFx& visualFx,
                                             const GoomEvents& goomEvents) noexcept
  : m_goomInfo{goomInfo}, m_visualFx{visualFx}, m_goomEvents{goomEvents}
{
}

auto GoomMusicLinesReactor::GetGoomLineResetSettings(const int farVal) const -> GoomLineSettings
{
  GoomLineSettings lineSettings;

  lineSettings.mode = m_goomEvents.GetRandomLineTypeEvent();

  switch (lineSettings.mode)
  {
    case LinesFx::LineType::CIRCLE:
      if (farVal)
      {
        lineSettings.params.line1 = 0.47F;
        lineSettings.params.line2 = lineSettings.params.line1;
        lineSettings.amplitude = 0.8F;
        break;
      }
      if (m_goomEvents.Happens(GoomEvent::CHANGE_LINE_CIRCLE_AMPLITUDE))
      {
        lineSettings.params.line1 = 0.0F;
        lineSettings.params.line2 = 0.0F;
        lineSettings.amplitude = 3.0F;
      }
      else if (m_goomEvents.Happens(GoomEvent::CHANGE_LINE_CIRCLE_PARAMS))
      {
        lineSettings.params.line1 = 0.40F * static_cast<float>(m_goomInfo.GetScreenInfo().height);
        lineSettings.params.line2 = 0.22F * static_cast<float>(m_goomInfo.GetScreenInfo().height);
        lineSettings.amplitude = 1.0F;
      }
      else
      {
        lineSettings.params.line1 = static_cast<float>(m_goomInfo.GetScreenInfo().height) * 0.35F;
        lineSettings.params.line2 = lineSettings.params.line1;
        lineSettings.amplitude = 1.0F;
      }
      break;
    case LinesFx::LineType::H_LINE:
      if (m_goomEvents.Happens(GoomEvent::CHANGE_H_LINE_PARAMS) || (farVal != 0))
      {
        lineSettings.params.line1 = static_cast<float>(m_goomInfo.GetScreenInfo().height) / 7.0F;
        lineSettings.params.line2 =
            (6.0F * static_cast<float>(m_goomInfo.GetScreenInfo().height)) / 7.0F;
        lineSettings.amplitude = 1.0F;
      }
      else
      {
        lineSettings.params.line1 = static_cast<float>(m_goomInfo.GetScreenInfo().height) / 2.0F;
        lineSettings.params.line2 = lineSettings.params.line1;
        lineSettings.amplitude = 2.0F;
      }
      break;
    case LinesFx::LineType::V_LINE:
      if (m_goomEvents.Happens(GoomEvent::CHANGE_V_LINE_PARAMS) || (farVal != 0))
      {
        lineSettings.params.line1 = static_cast<float>(m_goomInfo.GetScreenInfo().width) / 7.0F;
        lineSettings.params.line2 =
            (6.0F * static_cast<float>(m_goomInfo.GetScreenInfo().width)) / 7.0F;
        lineSettings.amplitude = 1.0F;
      }
      else
      {
        lineSettings.params.line1 = static_cast<float>(m_goomInfo.GetScreenInfo().width) / 2.0F;
        lineSettings.params.line2 = lineSettings.params.line1;
        lineSettings.amplitude = 1.5F;
      }
      break;
    default:
      throw std::logic_error("Unknown LineTypes enum.");
  }

  if ((farVal != 0) && m_goomEvents.Happens(GoomEvent::CHANGE_LINE_TO_BLACK))
  {
    lineSettings.colors.line1 = GetBlackLineColor();
    lineSettings.colors.line2 = lineSettings.colors.line1;
  }
  else
  {
    lineSettings.colors.line1 = m_visualFx.GetGoomLine1RandomColor();
    lineSettings.colors.line2 = m_visualFx.GetGoomLine2RandomColor();
  }

  return lineSettings;
}

/* arret aleatore.. changement de mode de ligne..
  */
void GoomMusicLinesReactor::StopRandomLineChangeMode()
{
  constexpr uint32_t DEC_LINE_MODE_CYCLES = 80;
  constexpr uint32_t UPDATE_LINE_MODE_CYCLES = 120;

  if (m_lineMode != m_drawLinesDuration)
  {
    --m_lineMode;
    if (-1 == m_lineMode)
    {
      m_lineMode = 0;
    }
  }
  else if ((0 == (m_updateNum % DEC_LINE_MODE_CYCLES)) &&
           m_goomEvents.Happens(GoomEvent::REDUCE_LINE_MODE) && (m_lineMode != 0))
  {
    --m_lineMode;
  }

  if ((0 == (m_updateNum % UPDATE_LINE_MODE_CYCLES)) &&
      m_goomEvents.Happens(GoomEvent::UPDATE_LINE_MODE) &&
      m_visualFx.IsCurrentlyDrawable(GoomDrawable::SCOPE))
  {
    if (0 == m_lineMode)
    {
      m_lineMode = m_drawLinesDuration;
    }
    else if ((m_lineMode == m_drawLinesDuration) && m_visualFx.CanResetDestGoomLines())
    {
      --m_lineMode;

      ResetGoomLines();
    }
  }
}

auto GoomMusicLinesReactor::GetNameValueParams() const -> NameValuePairs
{
  return {};
}

#if __cplusplus <= 201402L
} // namespace CONTROL
} // namespace GOOM
#else
} // namespace GOOM::CONTROL
#endif
