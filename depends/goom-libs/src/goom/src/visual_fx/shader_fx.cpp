#include "shader_fx.h"

//#undef NO_LOGGING

#include "color/colorutils.h"
#include "fx_helper.h"
#include "goom/spimpl.h"
#include "logging.h"
#include "shaders/high_contrast.h"
#include "utils/math/misc.h"
#include "utils/stopwatch.h"

#include <memory>

namespace GOOM::VISUAL_FX
{

using SHADERS::HighContrast;
using UTILS::Logging;
using UTILS::Stopwatch;
using UTILS::MATH::SMALL_FLOAT;
using UTILS::MATH::Sq;

class ShaderFx::ShaderFxImpl
{
public:
  explicit ShaderFxImpl(const FxHelper& fxHelper) noexcept;

  auto ChangeEffects() -> void;
  auto ApplyMultiple() -> void;
  auto ApplyEndEffect(const Stopwatch::TimeValues& timeValues) -> void;
  [[nodiscard]] auto GetLastShaderEffects() const -> const GoomShaderEffects&;

private:
  GoomShaderEffects m_goomShaderEffects{1.0F, HighContrast::DEFAULT_BRIGHTNESS,
                                        HighContrast::DEFAULT_CONTRAST};

  HighContrast m_highContrast;

  auto FadeToBlack(const Stopwatch::TimeValues& timeValues) -> void;
};

ShaderFx::ShaderFx(const FxHelper& fxHelper) noexcept
  : m_fxImpl{spimpl::make_unique_impl<ShaderFxImpl>(fxHelper)}
{
}

auto ShaderFx::GetFxName() const noexcept -> std::string
{
  return "shader";
}

auto ShaderFx::ChangeEffects() noexcept -> void
{
  m_fxImpl->ChangeEffects();
}

auto ShaderFx::Start() noexcept -> void
{
  // nothing to do
}

auto ShaderFx::Finish() noexcept -> void
{
  // nothing to do
}

auto ShaderFx::ApplyMultiple() noexcept -> void
{
  m_fxImpl->ApplyMultiple();
}

auto ShaderFx::ApplyEndEffect(const Stopwatch::TimeValues& timeValues) noexcept -> void
{
  m_fxImpl->ApplyEndEffect(timeValues);
}

auto ShaderFx::GetLastShaderEffects() const -> const GoomShaderEffects&
{
  return m_fxImpl->GetLastShaderEffects();
}

ShaderFx::ShaderFxImpl::ShaderFxImpl(const FxHelper& fxHelper) noexcept
  : m_highContrast{fxHelper.GetGoomInfo(), fxHelper.GetGoomRand()}
{
}

inline auto ShaderFx::ShaderFxImpl::ChangeEffects() -> void
{
  m_highContrast.ChangeHighContrast();
}

inline auto ShaderFx::ShaderFxImpl::ApplyMultiple() -> void
{
  m_highContrast.UpdateHighContrast();

  static constexpr float DEFAULT_EXPOSURE = 1.5F;
  m_goomShaderEffects.exposure = DEFAULT_EXPOSURE;
  m_goomShaderEffects.contrast = m_highContrast.GetCurrentContrast();
  m_goomShaderEffects.contrastMinChannelValue = m_highContrast.GetCurrentContrastMinChannelValue();
  m_goomShaderEffects.brightness = m_highContrast.GetCurrentBrightness();
}

inline auto ShaderFx::ShaderFxImpl::ApplyEndEffect(const Stopwatch::TimeValues& timeValues) -> void
{
  FadeToBlack(timeValues);
}

inline auto ShaderFx::ShaderFxImpl::GetLastShaderEffects() const -> const GoomShaderEffects&
{
  return m_goomShaderEffects;
}

inline auto ShaderFx::ShaderFxImpl::FadeToBlack(const Stopwatch::TimeValues& timeValues) -> void
{
  static constexpr float TIME_REMAINING_CUTOFF_IN_MS = 20000.0F;

  if (timeValues.timeRemainingInMs > TIME_REMAINING_CUTOFF_IN_MS)
  {
    return;
  }

  static constexpr float BRING_FINAL_BLACK_FORWARD_MS = 1000.0F;
  const float timeLeftAsFraction =
      std::max(0.0F, timeValues.timeRemainingInMs - BRING_FINAL_BLACK_FORWARD_MS) /
      TIME_REMAINING_CUTOFF_IN_MS;

  m_goomShaderEffects.brightness = Sq(timeLeftAsFraction);
}

} // namespace GOOM::VISUAL_FX
