#include "goom_music_lines_reactor.h"

#include "goom_events.h"
#include "utils/name_value_pairs.h"

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
using VISUAL_FX::LinesFx;

GoomMusicLinesReactor::GoomMusicLinesReactor(const PluginInfo& goomInfo,
                                             GoomAllVisualFx& visualFx,
                                             const GoomEvents& goomEvents) noexcept
  : m_goomInfo{goomInfo},
    m_visualFx{visualFx},
    m_goomEvents{goomEvents},
    m_screenWidth{static_cast<float>(m_goomInfo.GetScreenInfo().width)},
    m_screenHeight{static_cast<float>(m_goomInfo.GetScreenInfo().height)}
{
}

auto GoomMusicLinesReactor::GetGoomLineResetSettings(const int farVal) const -> GoomLineSettings
{
  switch (m_goomEvents.GetRandomLineTypeEvent())
  {
    case LinesFx::LineType::CIRCLE:
      return GetResetCircleLineSettings(farVal);
    case LinesFx::LineType::H_LINE:
      return GetResetHorizontalLineSettings(farVal);
    case LinesFx::LineType::V_LINE:
      return GetResetVerticalLineSettings(farVal);
    default:
      throw std::logic_error("Unknown LineTypes enum.");
  }
}

auto GoomMusicLinesReactor::GetResetCircleLineSettings(const int32_t farVal) const
    -> GoomLineSettings
{
  GoomLineSettings lineSettings;

  lineSettings.mode = LinesFx::LineType::CIRCLE;
  lineSettings.colors = GetResetLineColors(farVal);

  constexpr float NEW_FAR_VAL_PARAM1 = 0.47F;
  constexpr float NEW_FAR_VAL_PARAM2 = 0.47F;

  constexpr float NEW_NON_FAR_VAL_PARAM1_FACTOR = 0.40F;
  constexpr float NEW_NON_FAR_VAL_PARAM2_FACTOR = 0.22F;
  constexpr float DEFAULT_NON_FAR_VAL_PARAM1_FACTOR = 0.35F;

  constexpr float NEW_FAR_VAL_AMPLITUDE = 0.8F;
  constexpr float NEW_NON_FAR_VAL_AMPLITUDE = 3.0F;
  constexpr float DEFAULT_AMPLITUDE = 1.0F;

  if (farVal)
  {
    lineSettings.params.line1 = NEW_FAR_VAL_PARAM1;
    lineSettings.params.line2 = NEW_FAR_VAL_PARAM2;
    lineSettings.amplitude = NEW_FAR_VAL_AMPLITUDE;
  }
  else if (m_goomEvents.Happens(GoomEvent::CHANGE_LINE_CIRCLE_AMPLITUDE))
  {
    lineSettings.params.line1 = 0.0F;
    lineSettings.params.line2 = 0.0F;
    lineSettings.amplitude = NEW_NON_FAR_VAL_AMPLITUDE;
  }
  else if (m_goomEvents.Happens(GoomEvent::CHANGE_LINE_CIRCLE_PARAMS))
  {
    lineSettings.params.line1 = NEW_NON_FAR_VAL_PARAM1_FACTOR * m_screenHeight;
    lineSettings.params.line2 = NEW_NON_FAR_VAL_PARAM2_FACTOR * m_screenHeight;
    lineSettings.amplitude = DEFAULT_AMPLITUDE;
  }
  else
  {
    lineSettings.params.line1 = DEFAULT_NON_FAR_VAL_PARAM1_FACTOR * m_screenHeight;
    lineSettings.params.line2 = lineSettings.params.line1;
    lineSettings.amplitude = DEFAULT_AMPLITUDE;
  }

  return lineSettings;
}

auto GoomMusicLinesReactor::GetResetHorizontalLineSettings(const int32_t farVal) const
    -> GoomLineSettings
{
  GoomLineSettings lineSettings;

  lineSettings.mode = LinesFx::LineType::H_LINE;
  lineSettings.colors = GetResetLineColors(farVal);

  constexpr float NEW_PARAM1_FACTOR = 1.0F / 7.0F;
  constexpr float NEW_PARAM2_FACTOR = 6.0F / 7.0F;
  constexpr float DEFAULT_PARAM1_FACTOR = 1.0F / 2.0F;
  constexpr float DEFAULT_PARAM2_FACTOR = 1.0F / 2.0F;

  constexpr float NEW_AMPLITUDE = 1.0F;
  constexpr float DEFAULT_AMPLITUDE = 2.0F;

  if (m_goomEvents.Happens(GoomEvent::CHANGE_H_LINE_PARAMS) || (farVal != 0))
  {
    lineSettings.params.line1 = NEW_PARAM1_FACTOR * m_screenHeight;
    lineSettings.params.line2 = NEW_PARAM2_FACTOR * m_screenHeight;
    lineSettings.amplitude = NEW_AMPLITUDE;
  }
  else
  {
    lineSettings.params.line1 = DEFAULT_PARAM1_FACTOR * m_screenHeight;
    lineSettings.params.line2 = DEFAULT_PARAM2_FACTOR * m_screenHeight;
    lineSettings.amplitude = DEFAULT_AMPLITUDE;
  }

  return lineSettings;
}

auto GoomMusicLinesReactor::GetResetVerticalLineSettings(const int32_t farVal) const
    -> GoomLineSettings
{
  GoomLineSettings lineSettings;

  lineSettings.mode = LinesFx::LineType::V_LINE;
  lineSettings.colors = GetResetLineColors(farVal);

  constexpr float NEW_PARAM1_FACTOR = 1.0F / 7.0F;
  constexpr float NEW_PARAM2_FACTOR = 6.0F / 7.0F;
  constexpr float DEFAULT_PARAM1_FACTOR = 1.0F / 2.0F;
  constexpr float DEFAULT_PARAM2_FACTOR = 1.0F / 2.0F;

  constexpr float NEW_AMPLITUDE = 1.0F;
  constexpr float DEFAULT_AMPLITUDE = 1.5F;

  if (m_goomEvents.Happens(GoomEvent::CHANGE_V_LINE_PARAMS) || (farVal != 0))
  {
    lineSettings.params.line1 = NEW_PARAM1_FACTOR * m_screenWidth;
    lineSettings.params.line2 = NEW_PARAM2_FACTOR * m_screenWidth;
    lineSettings.amplitude = NEW_AMPLITUDE;
  }
  else
  {
    lineSettings.params.line1 = DEFAULT_PARAM1_FACTOR * m_screenWidth;
    lineSettings.params.line2 = DEFAULT_PARAM2_FACTOR * m_screenWidth;
    lineSettings.amplitude = DEFAULT_AMPLITUDE;
  }

  return lineSettings;
}

auto GoomMusicLinesReactor::GetResetLineColors(const int32_t farVal) const
    -> GoomLineSettings::Colors
{
  if ((farVal != 0) && m_goomEvents.Happens(GoomEvent::CHANGE_LINE_TO_BLACK))
  {
    return GetSameLineColors(LinesFx::GetBlackLineColor());
  }

  return {m_visualFx.GetGoomLine1RandomColor(), m_visualFx.GetGoomLine2RandomColor()};
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
      m_visualFx.IsCurrentlyDrawable(GoomDrawables::SCOPE))
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
