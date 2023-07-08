//#undef NO_LOGGING

#include "shader_fx.h"

#include "color/color_utils.h"
#include "fx_helper.h"
#include "goom_logger.h"
#include "shaders/chroma_factor_lerper.h"
#include "shaders/color_multiplier_lerper.h"
#include "shaders/high_contrast.h"
#include "shaders/hue_shift_lerper.h"
#include "spimpl.h"
#include "utils/math/misc.h"
#include "utils/stopwatch.h"

#include <algorithm>
#include <string>

namespace GOOM::VISUAL_FX
{

using SHADERS::ChromaFactorLerper;
using SHADERS::ColorMultiplierLerper;
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

  static constexpr uint32_t HUE_MIN_NUM_LERP_ON_STEPS = 25U;
  static constexpr uint32_t HUE_MAX_NUM_LERP_ON_STEPS = 100U;
  static constexpr uint32_t HUE_MIN_LERP_OFF_TIME     = 500U;
  static constexpr uint32_t HUE_MAX_LERP_OFF_TIME     = 5000U;
  HueShiftLerper m_hueShiftLerper;
  static constexpr auto MIN_CHROMA_FACTOR = 0.5F;
  static constexpr auto MAX_CHROMA_FACTOR = 5.0F;
  ChromaFactorLerper m_chromaFactorLerper;

  static constexpr auto MIN_BASE_COLOR_MULTIPLIER = 0.96F;
  static constexpr auto MAX_BASE_COLOR_MULTIPLIER = 1.00F;
  ColorMultiplierLerper m_baseColorMultiplierLerper;

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

auto ShaderFx::Start() noexcept -> void
{
  // nothing to do
}

auto ShaderFx::Finish() noexcept -> void
{
  // nothing to do
}

auto ShaderFx::ChangePixelBlender(
    [[maybe_unused]] const PixelBlenderParams& pixelBlenderParams) noexcept -> void
{
  // nothing to do.
}

auto ShaderFx::ChangeEffects() noexcept -> void
{
  m_pimpl->ChangeEffects();
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
    m_hueShiftLerper{*fxHelper.goomInfo,
                     *fxHelper.goomRand,
                     {
                         HUE_MIN_NUM_LERP_ON_STEPS,
                         HUE_MAX_NUM_LERP_ON_STEPS,
                         HUE_MIN_LERP_OFF_TIME,
                         HUE_MAX_LERP_OFF_TIME
                     }},
    m_chromaFactorLerper{
        *fxHelper.goomInfo, *fxHelper.goomRand, MIN_CHROMA_FACTOR, MAX_CHROMA_FACTOR},
    m_baseColorMultiplierLerper{*fxHelper.goomInfo,
                                *fxHelper.goomRand,
                                MIN_BASE_COLOR_MULTIPLIER,
                                MAX_BASE_COLOR_MULTIPLIER}
{
}

inline auto ShaderFx::ShaderFxImpl::ChangeEffects() -> void
{
  m_highContrast.ChangeHighContrast();
  m_hueShiftLerper.ChangeHue();
  m_chromaFactorLerper.ChangeChromaFactorRange();
  m_baseColorMultiplierLerper.ChangeMultiplierRange();
}

inline auto ShaderFx::ShaderFxImpl::ApplyMultiple() -> void
{
  m_highContrast.UpdateHighContrast();
  m_hueShiftLerper.Update();
  m_chromaFactorLerper.Update();
  m_baseColorMultiplierLerper.Update();

  m_goomShaderVariables.contrast = m_highContrast.GetCurrentContrast();
  m_goomShaderVariables.contrastMinChannelValue =
      m_highContrast.GetCurrentContrastMinChannelValue();
  m_goomShaderVariables.brightness          = m_highContrast.GetCurrentBrightness();
  m_goomShaderVariables.hueShift            = m_hueShiftLerper.GetHueShift();
  m_goomShaderVariables.chromaFactor        = m_chromaFactorLerper.GetChromaFactor();
  m_goomShaderVariables.baseColorMultiplier = m_baseColorMultiplierLerper.GetColorMultiplier();
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
