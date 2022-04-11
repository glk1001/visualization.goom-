#include "shader_fx.h"

#include "color/colorutils.h"
#include "fx_helper.h"
//#undef NO_LOGGING
#include "goom/spimpl.h"
#include "logging.h"
#include "shaders/high_contrast.h"

#include <memory>

namespace GOOM::VISUAL_FX
{

using SHADERS::HighContrast;
using UTILS::Logging;

class ShaderFx::ShaderFxImpl
{
public:
  explicit ShaderFxImpl(const FxHelper& fxHelper) noexcept;

  void Start();
  void ChangeEffects();
  void ApplyMultiple();
  [[nodiscard]] auto GetLastShaderEffects() const -> const GoomShaderEffects&;

private:
  GoomShaderEffects m_goomShaderEffects{1.0F, HighContrast::DEFAULT_BRIGHTNESS,
                                        HighContrast::DEFAULT_CONTRAST};

  HighContrast m_highContrast;
};

ShaderFx::ShaderFx(const FxHelper& fxHelper) noexcept
  : m_fxImpl{spimpl::make_unique_impl<ShaderFxImpl>(fxHelper)}
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

void ShaderFx::ChangeEffects()
{
  m_fxImpl->ChangeEffects();
}

void ShaderFx::ApplyMultiple()
{
  m_fxImpl->ApplyMultiple();
}

auto ShaderFx::GetLastShaderEffects() const -> const GoomShaderEffects&
{
  return m_fxImpl->GetLastShaderEffects();
}

ShaderFx::ShaderFxImpl::ShaderFxImpl(const FxHelper& fxHelper) noexcept
  : m_highContrast{fxHelper.GetGoomInfo(), fxHelper.GetGoomRand()}
{
}

inline void ShaderFx::ShaderFxImpl::Start()
{
  // nothing to do
}

void ShaderFx::ShaderFxImpl::ChangeEffects()
{
  m_highContrast.ChangeHighContrast();
}

inline void ShaderFx::ShaderFxImpl::ApplyMultiple()
{
  m_highContrast.UpdateHighContrast();

  static constexpr float DEFAULT_EXPOSURE = 2.0F;
  m_goomShaderEffects.exposure = DEFAULT_EXPOSURE;
  m_goomShaderEffects.contrast = m_highContrast.GetCurrentContrast();
  m_goomShaderEffects.contrastMinChannelValue = m_highContrast.GetCurrentContrastMinChannelValue();
  m_goomShaderEffects.brightness = m_highContrast.GetCurrentBrightness();
}

inline auto ShaderFx::ShaderFxImpl::GetLastShaderEffects() const -> const GoomShaderEffects&
{
  return m_goomShaderEffects;
}

} // namespace GOOM::VISUAL_FX
