#include "shader_fx.h"

#include "color/colorutils.h"
#include "draw/goom_draw.h"
#include "fx_helpers.h"
#include "goom_plugin_info.h"
#include "logging_control.h"
//#undef NO_LOGGING
#include "goom/spimpl.h"
#include "logging.h"
#include "utils/goom_rand_base.h"
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

using COLOR::GetLuma;
using DRAW::IGoomDraw;
using UTILS::IGoomRand;
using UTILS::Logging;
using UTILS::Timer;
using UTILS::TValue;

class ShaderFx::ShaderFxImpl
{
public:
  explicit ShaderFxImpl(const FxHelpers& fxHelpers) noexcept;

  void Start();
  void StartExposureControl();
  void ApplyMultiple();
  [[nodiscard]] auto GetLastShaderEffects() const -> const GoomShaderEffects&;

private:
  IGoomDraw& m_draw;
  const PluginInfo& m_goomInfo;
  IGoomRand& m_goomRand;
  uint64_t m_updateNum{};

  static constexpr float DEFAULT_EXPOSURE = 1.0F;
  static constexpr float DEFAULT_BRIGHTNESS = 1.0F;
  static constexpr float DEFAULT_CONTRAST = 1.0F;

  const IGoomRand::NumberRange<int32_t> m_exposureSampleWidthRange;
  const IGoomRand::NumberRange<int32_t> m_exposureSampleHeightRange;
  bool m_doExposureControl = false;
  void UpdateExposure();

  static constexpr uint32_t NUM_HIGH_CONTRAST_ON_STEPS = 250;
  static constexpr uint32_t HIGH_CONTRAST_ON_DELAY_TIME = 100;
  static constexpr uint32_t HIGH_CONTRAST_ON_TIME =
      (2 * NUM_HIGH_CONTRAST_ON_STEPS) + HIGH_CONTRAST_ON_DELAY_TIME;
  TValue m_highContrastT{TValue::StepType::CONTINUOUS_REVERSIBLE,
                         NUM_HIGH_CONTRAST_ON_STEPS,
                         {{1.0F, HIGH_CONTRAST_ON_DELAY_TIME}}};
  Timer m_highContrastOnTimer{HIGH_CONTRAST_ON_TIME, true};
  static constexpr uint32_t HIGH_CONTRAST_OFF_TIME = 300;
  Timer m_highContrastOffTimer{HIGH_CONTRAST_OFF_TIME, false};
  float m_contrastMinChannelValue = 0.0F;
  void UpdateHighContrast();

  GoomShaderEffects m_goomShaderEffects{1.5F * DEFAULT_EXPOSURE, DEFAULT_BRIGHTNESS,
                                        DEFAULT_CONTRAST};
  [[nodiscard]] auto GetAverageLuminanceOfSpotSamples() const -> float;
};

ShaderFx::ShaderFx(const FxHelpers& fxHelpers) noexcept
  : m_fxImpl{spimpl::make_unique_impl<ShaderFxImpl>(fxHelpers)}
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

void ShaderFx::StartExposureControl()
{
  m_fxImpl->StartExposureControl();
}

void ShaderFx::ApplyMultiple()
{
  m_fxImpl->ApplyMultiple();
}

auto ShaderFx::GetLastShaderEffects() const -> const GoomShaderEffects&
{
  return m_fxImpl->GetLastShaderEffects();
}

ShaderFx::ShaderFxImpl::ShaderFxImpl(const FxHelpers& fxHelpers) noexcept
  : m_draw{fxHelpers.GetDraw()},
    m_goomInfo{fxHelpers.GetGoomInfo()},
    m_goomRand{fxHelpers.GetGoomRand()},
    m_exposureSampleWidthRange{
        static_cast<int32_t>(m_goomInfo.GetScreenInfo().width / 3),
        static_cast<int32_t>(1 + ((2 * m_goomInfo.GetScreenInfo().width) / 3))},
    m_exposureSampleHeightRange{
        static_cast<int32_t>(m_goomInfo.GetScreenInfo().height / 3),
        static_cast<int32_t>(1 + ((2 * m_goomInfo.GetScreenInfo().height) / 3))}
{
}

inline void ShaderFx::ShaderFxImpl::Start()
{
  m_updateNum = 0;
  m_doExposureControl = false;
}

inline void ShaderFx::ShaderFxImpl::StartExposureControl()
{
  m_doExposureControl = true;
}

inline void ShaderFx::ShaderFxImpl::ApplyMultiple()
{
  ++m_updateNum;

  UpdateHighContrast();
  UpdateExposure();
}

inline void ShaderFx::ShaderFxImpl::UpdateHighContrast()
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
    m_goomShaderEffects.contrast = stdnew::lerp(DEFAULT_CONTRAST, HIGH_CONTRAST, m_highContrastT());
    m_goomShaderEffects.contrastMinChannelValue =
        stdnew::lerp(0.0F, m_contrastMinChannelValue, m_highContrastT());
    constexpr float CONTRAST_BRIGHTNESS = 0.5F;
    m_goomShaderEffects.brightness =
        stdnew::lerp(DEFAULT_BRIGHTNESS, CONTRAST_BRIGHTNESS, m_highContrastT());

    return;
  }

  if (m_highContrastOnTimer.JustFinished())
  {
    m_highContrastOffTimer.ResetToZero();
    return;
  }

  constexpr float PROB_CONTRAST = 0.5F;
  if ((0 == m_goomInfo.GetSoundInfo().GetTimeSinceLastGoom()) &&
      m_goomRand.ProbabilityOf(PROB_CONTRAST))
  {
    m_highContrastT.Reset();
    m_highContrastOnTimer.ResetToZero();
    constexpr float CONTRAST_MIN_CHAN = -0.1F;
    constexpr float PROB_ZERO_CONTRAST_MIN_CHAN = 0.8F;
    m_contrastMinChannelValue =
        m_goomRand.ProbabilityOf(PROB_ZERO_CONTRAST_MIN_CHAN) ? 0.0F : CONTRAST_MIN_CHAN;
  }
}

inline void ShaderFx::ShaderFxImpl::UpdateExposure()
{
  if (!m_doExposureControl)
  {
    return;
  }

  const float avLuminance = GetAverageLuminanceOfSpotSamples();
  constexpr float MIN_EXPOSURE = 0.01F;
  constexpr float MAX_EXPOSURE = 100.0F;
  const float targetExposure = stdnew::clamp(1.0F / avLuminance, MIN_EXPOSURE, MAX_EXPOSURE);

  constexpr float EXPOSURE_LERP_FACTOR = 0.1F;
  m_goomShaderEffects.exposure =
      stdnew::lerp(m_goomShaderEffects.exposure, targetExposure, EXPOSURE_LERP_FACTOR);
}

auto ShaderFx::ShaderFxImpl::GetAverageLuminanceOfSpotSamples() const -> float
{
  float totalLogLuminance = 0.0F;
  constexpr size_t NUM_SAMPLES_TO_DO = 10;
  size_t numNonZeroSamples = 0;
  for (size_t i = 0; i < NUM_SAMPLES_TO_DO; ++i)
  {
    const int32_t x = m_goomRand.GetRandInRange(m_exposureSampleWidthRange);
    const int32_t y = m_goomRand.GetRandInRange(m_exposureSampleHeightRange);
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
