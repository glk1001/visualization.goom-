#pragma once

#include "goom_plugin_info.h"
#include "utils/math/goom_rand.h"
#include "utils/t_values.h"
#include "utils/timer.h"

#include <array>
#include <cstdint>

namespace GOOM::VISUAL_FX::SHADERS
{

class HueShiftLerper
{
public:
  HueShiftLerper(const PluginInfo& goomInfo, const UTILS::MATH::IGoomRand& goomRand) noexcept;

  auto Update() noexcept -> void;

  [[nodiscard]] auto GetLerpT() const noexcept -> float;
  [[nodiscard]] auto GetSrceHueShift() const noexcept -> float;
  [[nodiscard]] auto GetDestHueShift() const noexcept -> float;

private:
  const PluginInfo& m_goomInfo;
  const UTILS::MATH::IGoomRand& m_goomRand;

  float m_srceHueShift = 0.0F;
  float m_destHueShift = 0.0F;

  static constexpr uint32_t MIN_NUM_LERP_ON_STEPS     = 50U;
  static constexpr uint32_t MAX_NUM_LERP_ON_STEPS     = 500U;
  static constexpr uint32_t DEFAULT_NUM_LERP_ON_STEPS = MIN_NUM_LERP_ON_STEPS;
  UTILS::TValue m_lerpT{UTILS::TValue::StepType::SINGLE_CYCLE, DEFAULT_NUM_LERP_ON_STEPS};

  static constexpr uint32_t MIN_LERP_OFF_TIME     = 500U;
  static constexpr uint32_t MAX_LERP_OFF_TIME     = 5000U;
  static constexpr uint32_t DEFAULT_LERP_OFF_TIME = MIN_LERP_OFF_TIME;
  UTILS::Timer m_lerpOffTimer{DEFAULT_LERP_OFF_TIME, false};

  [[nodiscard]] auto RestartLerp() const noexcept -> bool;
};

inline auto HueShiftLerper::GetLerpT() const noexcept -> float
{
  return m_lerpT();
}

inline auto HueShiftLerper::GetSrceHueShift() const noexcept -> float
{
  return m_srceHueShift;
}

inline auto HueShiftLerper::GetDestHueShift() const noexcept -> float
{
  return m_destHueShift;
}

} // namespace GOOM::VISUAL_FX::SHADERS
