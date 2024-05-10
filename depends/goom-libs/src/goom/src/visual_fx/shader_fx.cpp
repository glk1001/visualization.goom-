module;

//#undef NO_LOGGING

#include "goom/goom_config.h"

#include <algorithm>
#include <cstdint>
#include <string>

module Goom.VisualFx.ShaderFx;

import Goom.Utils.Math.Misc;
import Goom.VisualFx.FxHelper;
import Goom.Lib.FrameData;
import Goom.Lib.SPimpl;
import :ChromaFactorLerper;
import :ColorMultiplierLerper;
import :HighContrast;
import :HueShifterLerper;

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

  auto Start() noexcept -> void;

  auto ChangeEffects() -> void;
  auto SetFrameMiscData(MiscData& miscData) noexcept -> void;
  auto ApplyToImageBuffers() -> void;
  auto ApplyEndEffect(const Stopwatch::TimeValues& timeValues) -> void;

private:
  MiscData* m_frameMiscData = nullptr;

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
  m_pimpl->Start();
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

auto ShaderFx::SetFrameMiscData(MiscData& miscData) noexcept -> void
{
  m_pimpl->SetFrameMiscData(miscData);
}

auto ShaderFx::ApplyToImageBuffers() noexcept -> void
{
  m_pimpl->ApplyToImageBuffers();
}

auto ShaderFx::ApplyEndEffect(const Stopwatch::TimeValues& timeValues) noexcept -> void
{
  m_pimpl->ApplyEndEffect(timeValues);
}

ShaderFx::ShaderFxImpl::ShaderFxImpl(const FxHelper& fxHelper) noexcept
  : m_highContrast{fxHelper.GetGoomInfo(), fxHelper.GetGoomRand()},
    m_hueShiftLerper{fxHelper.GetGoomInfo(),
                     fxHelper.GetGoomRand(),
                     {
                         HUE_MIN_NUM_LERP_ON_STEPS,
                         HUE_MAX_NUM_LERP_ON_STEPS,
                         HUE_MIN_LERP_OFF_TIME,
                         HUE_MAX_LERP_OFF_TIME
                     }},
    m_chromaFactorLerper{
        fxHelper.GetGoomInfo(), fxHelper.GetGoomRand(), MIN_CHROMA_FACTOR, MAX_CHROMA_FACTOR},
    m_baseColorMultiplierLerper{fxHelper.GetGoomInfo(),
                                fxHelper.GetGoomRand(),
                                MIN_BASE_COLOR_MULTIPLIER,
                                MAX_BASE_COLOR_MULTIPLIER}
{
}

inline auto ShaderFx::ShaderFxImpl::Start() noexcept -> void
{
  m_frameMiscData = nullptr;
}

inline auto ShaderFx::ShaderFxImpl::ChangeEffects() -> void
{
  m_highContrast.ChangeHighContrast();
  m_hueShiftLerper.ChangeHue();
  m_chromaFactorLerper.ChangeChromaFactorRange();
  m_baseColorMultiplierLerper.ChangeMultiplierRange();
}

inline auto ShaderFx::ShaderFxImpl::SetFrameMiscData(MiscData& miscData) noexcept -> void
{
  m_frameMiscData = &miscData;
}

inline auto ShaderFx::ShaderFxImpl::ApplyToImageBuffers() -> void
{
  Expects(m_frameMiscData != nullptr);

  m_highContrast.UpdateHighContrast();
  m_hueShiftLerper.Update();
  m_chromaFactorLerper.Update();
  m_baseColorMultiplierLerper.Update();

  m_frameMiscData->brightness          = m_highContrast.GetCurrentBrightness();
  m_frameMiscData->hueShift            = m_hueShiftLerper.GetHueShift();
  m_frameMiscData->chromaFactor        = m_chromaFactorLerper.GetChromaFactor();
  m_frameMiscData->baseColorMultiplier = m_baseColorMultiplierLerper.GetColorMultiplier();
}

inline auto ShaderFx::ShaderFxImpl::ApplyEndEffect(const Stopwatch::TimeValues& timeValues) -> void
{
  FadeToBlack(timeValues);
}

inline auto ShaderFx::ShaderFxImpl::FadeToBlack(const Stopwatch::TimeValues& timeValues) -> void
{
  Expects(m_frameMiscData != nullptr);

  static constexpr auto TIME_REMAINING_CUTOFF_IN_MS = 20000.0F;

  if (timeValues.timeRemainingInMs > TIME_REMAINING_CUTOFF_IN_MS)
  {
    return;
  }

  static constexpr auto BRING_FINAL_BLACK_FORWARD_MS = 1000.0F;
  const auto timeLeftAsFraction =
      std::max(0.0F, timeValues.timeRemainingInMs - BRING_FINAL_BLACK_FORWARD_MS) /
      TIME_REMAINING_CUTOFF_IN_MS;

  m_frameMiscData->brightness = Sq(timeLeftAsFraction);
}

} // namespace GOOM::VISUAL_FX
