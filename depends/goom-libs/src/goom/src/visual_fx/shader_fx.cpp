#include "shader_fx.h"

#include "draw/goom_draw.h"
#include "goom_plugin_info.h"
#include "logging_control.h"
//#undef NO_LOGGING
#include "goom/spimpl.h"
#include "logging.h"
#include "utils/goomrand.h"
#include "utils/t_values.h"
#include "utils/timer.h"
#include "v2d.h"

#undef NDEBUG
#include <cassert>
#include <memory>

#if __cplusplus <= 201402L
namespace GOOM
{
namespace VISUAL_FX
{
#else
namespace GOOM::VISUAL_FX
{
#endif

using DRAW::IGoomDraw;
using UTILS::GetRandInRange;
using UTILS::Logging;
using UTILS::NumberRange;
using UTILS::ProbabilityOf;
using UTILS::Timer;
using UTILS::TValue;

class ShaderFx::ShaderFxImpl
{
public:
  ShaderFxImpl(DRAW::IGoomDraw& draw, const PluginInfo& goomInfo) noexcept;

  void Start();
  void ApplyMultiple();
  [[nodiscard]] auto GetLastShaderEffects() const -> const GoomShaderEffects&;

private:
  IGoomDraw& m_draw;
  const PluginInfo& m_goomInfo;
  uint64_t m_updateNum{};

  const NumberRange<int32_t> m_exposureSampleWidthRange;
  const NumberRange<int32_t> m_exposureSampleHeightRange;
  static constexpr uint32_t NUM_UPDATES_BETWEEN_EXPOSURE_CHECKS = 5;
  TValue m_exposureChangeT{TValue::StepType::SINGLE_CYCLE, NUM_UPDATES_BETWEEN_EXPOSURE_CHECKS};
  static constexpr float INITIAL_BRIGHTNESS = 2.0F;
  static constexpr float INITIAL_EXPOSURE = 2.0F;
  float m_targetExposure = INITIAL_EXPOSURE;
  void UpdateExposure();


  static constexpr uint32_t NUM_HIGH_BRIGHTNESS_STEPS = 100;
  TValue m_highBrightnessT{TValue::StepType::SINGLE_CYCLE, NUM_HIGH_BRIGHTNESS_STEPS, 1.0F};
  void UpdateHighBrightness();

  static constexpr uint32_t NUM_HIGH_CONTRAST_STEPS = 250;
  static constexpr uint32_t HIGH_CONTRAST_DELAY_TIME = 100;
  static constexpr uint32_t TOTAL_HIGH_CONTRAST_STEPS =
      (2 * NUM_HIGH_CONTRAST_STEPS) + HIGH_CONTRAST_DELAY_TIME;
  TValue m_highContrastT{TValue::StepType::CONTINUOUS_REVERSIBLE,
                         NUM_HIGH_CONTRAST_STEPS,
                         {{1.0F, HIGH_CONTRAST_DELAY_TIME}}};
  Timer m_highContrastTimer{TOTAL_HIGH_CONTRAST_STEPS, true};
  static constexpr float INITIAL_CONTRAST = 1.0F;
  void UpdateHighContrast();

  GoomShaderEffects m_goomShaderEffects{INITIAL_EXPOSURE, INITIAL_BRIGHTNESS, INITIAL_CONTRAST};
  [[nodiscard]] auto GetAverageLuminanceOfSpotSamples() const -> float;
};

ShaderFx::ShaderFx(IGoomDraw& draw, const PluginInfo& goomInfo) noexcept
  : m_fxImpl{spimpl::make_unique_impl<ShaderFxImpl>(draw, goomInfo)}
{
}

void ShaderFx::Start()
{
  m_fxImpl->Start();
}

void ShaderFx::Finish()
{
  // nothing to do
}

auto ShaderFx::GetFxName() const -> std::string
{
  return "shader";
}

void ShaderFx::ApplyMultiple()
{
  m_fxImpl->ApplyMultiple();
}

auto ShaderFx::GetLastShaderEffects() const -> const GoomShaderEffects&
{
  return m_fxImpl->GetLastShaderEffects();
}

ShaderFx::ShaderFxImpl::ShaderFxImpl(IGoomDraw& draw, const PluginInfo& goomInfo) noexcept
  : m_draw{draw},
    m_goomInfo{goomInfo},
    m_exposureSampleWidthRange{static_cast<int32_t>(m_goomInfo.GetScreenInfo().width / 3),
                 static_cast<int32_t>(1 + ((2 * m_goomInfo.GetScreenInfo().width) / 3))},
    m_exposureSampleHeightRange{static_cast<int32_t>(m_goomInfo.GetScreenInfo().height / 3),
                  static_cast<int32_t>(1 + ((2 * m_goomInfo.GetScreenInfo().height) / 3))}
{
}

inline void ShaderFx::ShaderFxImpl::Start()
{
  m_updateNum = 0;
}

inline void ShaderFx::ShaderFxImpl::ApplyMultiple()
{
  ++m_updateNum;

  UpdateHighBrightness();
  UpdateHighContrast();
  UpdateExposure();
}

inline void ShaderFx::ShaderFxImpl::UpdateHighBrightness()
{
  m_highBrightnessT.Increment();

  if ((0 == m_goomInfo.GetSoundInfo().GetTimeSinceLastBigGoom()) && ProbabilityOf(0.0F))
  {
    m_highBrightnessT.Reset();
  }
  constexpr float MIX_VAL = 0.3F;
  m_goomShaderEffects.brightness = stdnew::lerp(
      m_goomShaderEffects.brightness, 1.0F + ((1.0F - m_highBrightnessT()) * 4.0F), MIX_VAL);
}

inline void ShaderFx::ShaderFxImpl::UpdateHighContrast()
{
  m_highContrastT.Increment();
  m_highContrastTimer.Increment();

  if ((0 == m_goomInfo.GetSoundInfo().GetTimeSinceLastGoom()) && ProbabilityOf(0.5F) &&
      m_highContrastTimer.Finished())
  {
    m_highContrastT.Reset();
    m_highContrastTimer.ResetToZero();
  }

  if (!m_highContrastTimer.Finished())
  {
    m_goomShaderEffects.contrast =
        stdnew::lerp(m_goomShaderEffects.contrast, 1.01F, m_highContrastT());
  }
}

inline void ShaderFx::ShaderFxImpl::UpdateExposure()
{
  if (0 == (m_updateNum % NUM_UPDATES_BETWEEN_EXPOSURE_CHECKS))
  {
    const float avLuminance = GetAverageLuminanceOfSpotSamples();
    constexpr float MIN_EXPOSURE = 1.0F;
    constexpr float MAX_EXPOSURE = 10.0F;
    m_targetExposure = stdnew::clamp(1.0F / avLuminance, MIN_EXPOSURE, MAX_EXPOSURE);
    m_exposureChangeT.Reset();

    /**
    constexpr float TOO_BRIGHT = 0.95F;
    if (avLuminance > TOO_BRIGHT)
    {
      // Stop any high brightness.
      m_highBrightnessT.Reset(1.0F);
    }
     ***/
  }
  m_goomShaderEffects.exposure =
      stdnew::lerp(m_goomShaderEffects.exposure, m_targetExposure, m_exposureChangeT());
}

[[nodiscard]] inline auto GetLuma(const Pixel& color) -> float
{
  return (0.2125F * color.RFlt()) + (0.7154F * color.GFlt()) + (0.0721F * color.BFlt());
}

auto ShaderFx::ShaderFxImpl::GetAverageLuminanceOfSpotSamples() const -> float
{
  float totalLogLuminance = 0.0F;
  constexpr size_t NUM_SAMPLES = 10;
  size_t numNonZeroSamples = 0;
  for (size_t i = 0; i < NUM_SAMPLES; ++i)
  {
    const int32_t x = GetRandInRange(m_exposureSampleWidthRange);
    const int32_t y = GetRandInRange(m_exposureSampleHeightRange);
    const Pixel pixel = m_draw.GetPixel(x, y);
    const float luma = GetLuma(pixel);
    if (luma > 0.0F)
    {
      totalLogLuminance += std::log(luma);
      ++numNonZeroSamples;
    }
  }

  if (0 == numNonZeroSamples)
  {
    constexpr float SMALL_LUM = 0.01F;
    return SMALL_LUM;
  }

  return std::exp(totalLogLuminance / static_cast<float>(numNonZeroSamples));
}

inline auto ShaderFx::ShaderFxImpl::GetLastShaderEffects() const -> const GoomShaderEffects&
{
  return m_goomShaderEffects;
}

#if __cplusplus <= 201402L
} // namespace VISUAL_FX
} // namespace GOOM
#else
} // namespace GOOM::VISUAL_FX
#endif
