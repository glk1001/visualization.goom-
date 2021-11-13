#include "high_contrast.h"


#if __cplusplus <= 201402L
namespace GOOM
{
namespace VISUAL_FX
{
namespace SHADERS
{
#else
namespace GOOM::VISUAL_FX::SHADERS
{
#endif

using UTILS::IGoomRand;
using UTILS::Timer;
using UTILS::TValue;

HighContrast::HighContrast(const PluginInfo& goomInfo, IGoomRand& goomRand) noexcept
  : m_goomInfo{goomInfo}, m_goomRand{goomRand}
{
}

void HighContrast::Start()
{
  m_highContrastT.Reset();
  m_highContrastOnTimer.ResetToZero();
  m_highContrastOffTimer.ResetToZero();

  m_currentContrast = DEFAULT_CONTRAST;
  m_currentBrightness = DEFAULT_BRIGHTNESS;
  m_currentContrastMinChannelValue = 0.0F;
  m_maxContrastMinChannelValue = 0.0F;
}

void HighContrast::UpdateHighContrast()
{
  m_highContrastT.Increment();
  m_highContrastOnTimer.Increment();
  m_highContrastOffTimer.Increment();

  if (!m_highContrastOffTimer.Finished())
  {
    return;
  }

  if (!m_highContrastOnTimer.Finished())
  {
    constexpr float HIGH_CONTRAST = 1.01F;
    m_currentContrast = stdnew::lerp(DEFAULT_CONTRAST, HIGH_CONTRAST, m_highContrastT());
    m_currentContrastMinChannelValue =
        stdnew::lerp(0.0F, m_maxContrastMinChannelValue, m_highContrastT());
    constexpr float CONTRAST_BRIGHTNESS = 0.9F;
    m_currentBrightness = stdnew::lerp(DEFAULT_BRIGHTNESS, CONTRAST_BRIGHTNESS, m_highContrastT());

    return;
  }

  if (m_highContrastOnTimer.JustFinished())
  {
    m_highContrastOffTimer.ResetToZero();
    return;
  }

  constexpr float PROB_CONTRAST = 0.3F;
  if ((0 == m_goomInfo.GetSoundInfo().GetTimeSinceLastGoom()) &&
      m_goomRand.ProbabilityOf(PROB_CONTRAST))
  {
    m_highContrastT.Reset();
    m_highContrastOnTimer.ResetToZero();
    constexpr float CONTRAST_MIN_CHAN = -0.4F;
    constexpr float PROB_ZERO_CONTRAST_MIN_CHAN = 0.9F;
    m_maxContrastMinChannelValue =
        m_goomRand.ProbabilityOf(PROB_ZERO_CONTRAST_MIN_CHAN) ? 0.0F : CONTRAST_MIN_CHAN;
  }
}

#if __cplusplus <= 201402L
} // namespace SHADERS
} // namespace VISUAL_FX
} // namespace GOOM
#else
} // namespace GOOM::VISUAL_FX::SHADERS
#endif
