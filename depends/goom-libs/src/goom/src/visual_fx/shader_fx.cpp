//#undef NO_LOGGING

#include "shader_fx.h"

#include "color/color_utils.h"
#include "fx_helper.h"
#include "goom_logger.h"
#include "shaders/high_contrast.h"
#include "shaders/hue_shift_lerper.h"
#include "spimpl.h"
#include "utils/math/misc.h"
#include "utils/stopwatch.h"

#include <algorithm>
#include <string>

namespace GOOM::VISUAL_FX
{

using SHADERS::HighContrast;
using SHADERS::HueShiftLerper;
using UTILS::Stopwatch;
using UTILS::MATH::Sq;

class ShaderFx::ShaderFxImpl
{
public:
  explicit ShaderFxImpl(const FxHelper& fxHelper) noexcept;

  auto ChangeEffects() -> void;
  auto ApplyMultiple() -> void;
  auto ApplyEndEffect(const Stopwatch::TimeValues& timeValues) -> void;
  [[nodiscard]] auto GetLastShaderVariables() const -> const GoomShaderVariables&;

private:
  GoomShaderVariables m_goomShaderVariables{
      1.0F, HighContrast::DEFAULT_BRIGHTNESS, HighContrast::DEFAULT_CONTRAST, 0.0F};

  HighContrast m_highContrast;
  HueShiftLerper m_hueShiftLerper;

  auto FadeToBlack(const Stopwatch::TimeValues& timeValues) -> void;
};

ShaderFx::ShaderFx(const FxHelper& fxHelper) noexcept
  : m_pimpl{spimpl::make_unique_impl<ShaderFxImpl>(fxHelper)}
{
}

auto ShaderFx::GetFxName() const noexcept -> std::string
{
  return "shader";
}

auto ShaderFx::ChangeEffects() noexcept -> void
{
  m_pimpl->ChangeEffects();
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
  m_pimpl->ApplyMultiple();
}

auto ShaderFx::ApplyEndEffect(const Stopwatch::TimeValues& timeValues) noexcept -> void
{
  m_pimpl->ApplyEndEffect(timeValues);
}

auto ShaderFx::GetLastShaderVariables() const -> const GoomShaderVariables&
{
  return m_pimpl->GetLastShaderVariables();
}

ShaderFx::ShaderFxImpl::ShaderFxImpl(const FxHelper& fxHelper) noexcept
  : m_highContrast{*fxHelper.goomInfo, *fxHelper.goomRand},
    m_hueShiftLerper{*fxHelper.goomInfo, *fxHelper.goomRand}
{
}

inline auto ShaderFx::ShaderFxImpl::ChangeEffects() -> void
{
  m_highContrast.ChangeHighContrast();
  m_hueShiftLerper.ChangeHue();
}

inline auto ShaderFx::ShaderFxImpl::ApplyMultiple() -> void
{
  m_highContrast.UpdateHighContrast();
  m_hueShiftLerper.Update();

  static constexpr auto DEFAULT_EXPOSURE = 1.5F;
  m_goomShaderVariables.exposure         = DEFAULT_EXPOSURE;
  m_goomShaderVariables.contrast         = m_highContrast.GetCurrentContrast();
  m_goomShaderVariables.contrastMinChannelValue =
      m_highContrast.GetCurrentContrastMinChannelValue();
  m_goomShaderVariables.brightness    = m_highContrast.GetCurrentBrightness();
  m_goomShaderVariables.hueShiftLerpT = m_hueShiftLerper.GetLerpT();
  m_goomShaderVariables.srceHueShift  = m_hueShiftLerper.GetSrceHueShift();
  m_goomShaderVariables.destHueShift  = m_hueShiftLerper.GetDestHueShift();
}

inline auto ShaderFx::ShaderFxImpl::ApplyEndEffect(const Stopwatch::TimeValues& timeValues) -> void
{
  FadeToBlack(timeValues);
}

inline auto ShaderFx::ShaderFxImpl::GetLastShaderVariables() const -> const GoomShaderVariables&
{
  return m_goomShaderVariables;
}

inline auto ShaderFx::ShaderFxImpl::FadeToBlack(const Stopwatch::TimeValues& timeValues) -> void
{
  static constexpr auto TIME_REMAINING_CUTOFF_IN_MS = 20000.0F;

  if (timeValues.timeRemainingInMs > TIME_REMAINING_CUTOFF_IN_MS)
  {
    return;
  }

  static constexpr auto BRING_FINAL_BLACK_FORWARD_MS = 1000.0F;
  const auto timeLeftAsFraction =
      std::max(0.0F, timeValues.timeRemainingInMs - BRING_FINAL_BLACK_FORWARD_MS) /
      TIME_REMAINING_CUTOFF_IN_MS;

  m_goomShaderVariables.brightness = Sq(timeLeftAsFraction);
}

} // namespace GOOM::VISUAL_FX
