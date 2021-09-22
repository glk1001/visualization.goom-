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

  lineSettings.amplitude = 1.0F;
  lineSettings.mode = m_goomEvents.GetRandomLineTypeEvent();

  switch (lineSettings.mode)
  {
    case LinesFx::LineType::CIRCLE:
      if (farVal)
      {
        lineSettings.line1.param = 0.47F;
        lineSettings.line2.param = lineSettings.line1.param;
        lineSettings.amplitude = 0.8F;
        break;
      }
      if (m_goomEvents.Happens(GoomEvent::CHANGE_LINE_CIRCLE_AMPLITUDE))
      {
        lineSettings.line1.param = 0.0F;
        lineSettings.line2.param = 0.0F;
        lineSettings.amplitude = 3.0F;
      }
      else if (m_goomEvents.Happens(GoomEvent::CHANGE_LINE_CIRCLE_PARAMS))
      {
        lineSettings.line1.param = 0.40F * static_cast<float>(m_goomInfo.GetScreenInfo().height);
        lineSettings.line2.param = 0.22F * static_cast<float>(m_goomInfo.GetScreenInfo().height);
      }
      else
      {
        lineSettings.line1.param = static_cast<float>(m_goomInfo.GetScreenInfo().height) * 0.35F;
        lineSettings.line2.param = lineSettings.line1.param;
      }
      break;
    case LinesFx::LineType::H_LINE:
      if (m_goomEvents.Happens(GoomEvent::CHANGE_H_LINE_PARAMS) || (farVal != 0))
      {
        lineSettings.line1.param = static_cast<float>(m_goomInfo.GetScreenInfo().height) / 7.0F;
        lineSettings.line2.param =
            (6.0F * static_cast<float>(m_goomInfo.GetScreenInfo().height)) / 7.0F;
      }
      else
      {
        lineSettings.line1.param = static_cast<float>(m_goomInfo.GetScreenInfo().height) / 2.0F;
        lineSettings.line2.param = lineSettings.line1.param;
        lineSettings.amplitude = 2.0F;
      }
      break;
    case LinesFx::LineType::V_LINE:
      if (m_goomEvents.Happens(GoomEvent::CHANGE_V_LINE_PARAMS) || (farVal != 0))
      {
        lineSettings.line1.param = static_cast<float>(m_goomInfo.GetScreenInfo().width) / 7.0F;
        lineSettings.line2.param =
            (6.0F * static_cast<float>(m_goomInfo.GetScreenInfo().width)) / 7.0F;
      }
      else
      {
        lineSettings.line1.param = static_cast<float>(m_goomInfo.GetScreenInfo().width) / 2.0F;
        lineSettings.line2.param = lineSettings.line1.param;
        lineSettings.amplitude = 1.5F;
      }
      break;
    default:
      throw std::logic_error("Unknown LineTypes enum.");
  }

  if ((farVal != 0) && m_goomEvents.Happens(GoomEvent::CHANGE_LINE_TO_BLACK))
  {
    lineSettings.line1.color = GetBlackLineColor();
    lineSettings.line2.color = lineSettings.line1.color;
  }
  else
  {
    lineSettings.line1.color = m_visualFx.GetGoomLine1RandomColor();
    lineSettings.line2.color = m_visualFx.GetGoomLine2RandomColor();
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
