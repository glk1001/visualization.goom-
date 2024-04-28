module;

#include "goom_plugin_info.h"
#include "utils/math/goom_rand_base.h"

#include <cmath>
#include <cstdint>

module Goom.VisualFx.ShaderFx:HighContrast;

import Goom.Utils;

namespace GOOM::VISUAL_FX::SHADERS
{

class HighContrast
{
public:
  static constexpr float DEFAULT_CONTRAST   = 1.0F;
  static constexpr float DEFAULT_BRIGHTNESS = 1.0F;

  HighContrast(const PluginInfo& goomInfo, const UTILS::MATH::IGoomRand& goomRand) noexcept;

  void Start();

  void ChangeHighContrast();
  void UpdateHighContrast();
  [[nodiscard]] auto GetCurrentContrast() const -> float;
  [[nodiscard]] auto GetCurrentBrightness() const -> float;
  [[nodiscard]] auto GetCurrentContrastMinChannelValue() const -> float;

private:
  const PluginInfo* m_goomInfo;
  const UTILS::MATH::IGoomRand* m_goomRand;

  float m_currentContrast                = DEFAULT_CONTRAST;
  float m_currentBrightness              = DEFAULT_BRIGHTNESS;
  float m_currentContrastMinChannelValue = 0.0F;
  float m_maxContrastMinChannelValue     = 0.0F;
  void ResetValues();

  static constexpr uint32_t NUM_HIGH_CONTRAST_ON_STEPS  = 250;
  static constexpr uint32_t HIGH_CONTRAST_ON_DELAY_TIME = 100;
  static constexpr uint32_t HIGH_CONTRAST_ON_TIME =
      (2 * NUM_HIGH_CONTRAST_ON_STEPS) + HIGH_CONTRAST_ON_DELAY_TIME;
  UTILS::TValue m_highContrastT{
      {UTILS::TValue::StepType::CONTINUOUS_REVERSIBLE, NUM_HIGH_CONTRAST_ON_STEPS},
      {{1.0F, HIGH_CONTRAST_ON_DELAY_TIME}}
  };
  UTILS::Timer m_highContrastOnTimer{m_goomInfo->GetTime(), HIGH_CONTRAST_ON_TIME, true};
  static constexpr uint32_t HIGH_CONTRAST_OFF_TIME = 300;
  UTILS::Timer m_highContrastOffTimer{m_goomInfo->GetTime(), HIGH_CONTRAST_OFF_TIME, false};
};

inline auto HighContrast::GetCurrentContrast() const -> float
{
  return m_currentContrast;
}

inline auto HighContrast::GetCurrentBrightness() const -> float
{
  return m_currentBrightness;
}

inline auto HighContrast::GetCurrentContrastMinChannelValue() const -> float
{
  return m_currentContrastMinChannelValue;
}

} // namespace GOOM::VISUAL_FX::SHADERS

namespace GOOM::VISUAL_FX::SHADERS
{

using UTILS::MATH::IGoomRand;

HighContrast::HighContrast(const PluginInfo& goomInfo, const IGoomRand& goomRand) noexcept
  : m_goomInfo{&goomInfo}, m_goomRand{&goomRand}
{
}

void HighContrast::Start()
{
  m_highContrastT.Reset();
  m_highContrastOnTimer.ResetToZero();
  m_highContrastOffTimer.ResetToZero();

  ResetValues();
}

inline void HighContrast::ResetValues()
{
  m_currentContrast                = DEFAULT_CONTRAST;
  m_currentBrightness              = DEFAULT_BRIGHTNESS;
  m_currentContrastMinChannelValue = 0.0F;
  m_maxContrastMinChannelValue     = 0.0F;
}

void HighContrast::ChangeHighContrast()
{
  if (!m_highContrastOffTimer.Finished())
  {
    return;
  }
  if (!m_highContrastOnTimer.Finished())
  {
    return;
  }

  if (static constexpr float PROB_CONTRAST = 0.001F;
      (0 == m_goomInfo->GetSoundEvents().GetTimeSinceLastGoom()) &&
      m_goomRand->ProbabilityOf(PROB_CONTRAST))
  {
    m_highContrastT.Reset();
    m_highContrastOnTimer.ResetToZero();
    static constexpr auto MIN_CONTRAST_MIN_CHAN = -0.5F;
    static constexpr auto MAX_CONTRAST_MIN_CHAN = -0.2F;
    m_maxContrastMinChannelValue =
        m_goomRand->GetRandInRange(MIN_CONTRAST_MIN_CHAN, MAX_CONTRAST_MIN_CHAN);
  }
}

void HighContrast::UpdateHighContrast()
{
  m_highContrastT.Increment();

  if (!m_highContrastOffTimer.Finished())
  {
    return;
  }

  if (!m_highContrastOnTimer.Finished())
  {
    static constexpr auto HIGH_CONTRAST = 1.01F;
    m_currentContrast = std::lerp(DEFAULT_CONTRAST, HIGH_CONTRAST, m_highContrastT());
    m_currentContrastMinChannelValue =
        std::lerp(0.0F, m_maxContrastMinChannelValue, m_highContrastT());
    static constexpr auto CONTRAST_BRIGHTNESS = 1.1F;
    m_currentBrightness = std::lerp(DEFAULT_BRIGHTNESS, CONTRAST_BRIGHTNESS, m_highContrastT());

    return;
  }

  if (m_highContrastOnTimer.JustFinished())
  {
    m_highContrastOffTimer.ResetToZero();
    ResetValues();
    return;
  }

  ChangeHighContrast();
}

} // namespace GOOM::VISUAL_FX::SHADERS
