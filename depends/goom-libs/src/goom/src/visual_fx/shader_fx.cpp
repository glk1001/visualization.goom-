#include "shader_fx.h"

#include "color/colorutils.h"
#include "draw/goom_draw.h"
#include "fx_helpers.h"
#include "goom_plugin_info.h"
#include "logging_control.h"
//#undef NO_LOGGING
#include "goom/spimpl.h"
#include "logging.h"
#include "shaders/adaptive_exposure.h"
#include "shaders/high_contrast.h"
#include "utils/goom_rand_base.h"

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
using SHADERS::AdaptiveExposure;
using SHADERS::HighContrast;
using UTILS::IGoomRand;
using UTILS::Logging;

class ShaderFx::ShaderFxImpl
{
public:
  explicit ShaderFxImpl(const FxHelpers& fxHelpers) noexcept;

  void Start();
  void StartExposureControl();
  void SetAverageLuminance(float value);
  void ApplyMultiple();
  [[nodiscard]] auto GetLastShaderEffects() const -> const GoomShaderEffects&;

private:
  IGoomDraw& m_draw;
  const PluginInfo& m_goomInfo;
  IGoomRand& m_goomRand;

  GoomShaderEffects m_goomShaderEffects{1.5F * AdaptiveExposure::DEFAULT_EXPOSURE,
                                        HighContrast::DEFAULT_BRIGHTNESS,
                                        HighContrast::DEFAULT_CONTRAST};

  float m_averageLuminance = 0.0F;
  bool m_doExposureControl = false;
  AdaptiveExposure m_adaptiveExposure{};
  void UpdateExposure();

  HighContrast m_highContrast;
  void UpdateHighContrast();
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

void ShaderFx::SetAverageLuminance(const float value)
{
  m_fxImpl->SetAverageLuminance(value);
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
    m_highContrast{m_goomInfo, m_goomRand}
{
}

inline void ShaderFx::ShaderFxImpl::Start()
{
  m_doExposureControl = false;
  m_adaptiveExposure.Start();
}

inline void ShaderFx::ShaderFxImpl::StartExposureControl()
{
  m_doExposureControl = true;
}

inline void ShaderFx::ShaderFxImpl::SetAverageLuminance(const float value)
{
  m_averageLuminance = value;
}

inline void ShaderFx::ShaderFxImpl::ApplyMultiple()
{
  UpdateHighContrast();
  UpdateExposure();
}

inline auto ShaderFx::ShaderFxImpl::GetLastShaderEffects() const -> const GoomShaderEffects&
{
  return m_goomShaderEffects;
}

inline void ShaderFx::ShaderFxImpl::UpdateHighContrast()
{
  m_highContrast.UpdateHighContrast();

  m_goomShaderEffects.contrast = m_highContrast.GetCurrentContrast();
  m_goomShaderEffects.contrastMinChannelValue = m_highContrast.GetCurrentContrastMinChannelValue();
  m_goomShaderEffects.brightness = m_highContrast.GetCurrentBrightness();
}

inline void ShaderFx::ShaderFxImpl::UpdateExposure()
{
  if (!m_doExposureControl)
  {
    return;
  }

  m_adaptiveExposure.UpdateExposure(m_averageLuminance);

  m_goomShaderEffects.exposure = m_adaptiveExposure.GetCurrentExposure();
}

#if __cplusplus <= 201402L
} // namespace VISUAL_FX
} // namespace GOOM
#else
} // namespace GOOM::VISUAL_FX
#endif
