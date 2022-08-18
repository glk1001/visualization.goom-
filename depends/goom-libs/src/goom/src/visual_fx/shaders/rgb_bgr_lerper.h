#pragma once

#include "goom_plugin_info.h"
#include "utils/math/goom_rand.h"
#include "utils/t_values.h"
#include "utils/timer.h"

#include <array>
#include <cstdint>

namespace GOOM::VISUAL_FX::SHADERS
{

class RgbBgrLerper
{
public:
  RgbBgrLerper(const PluginInfo& goomInfo,
               const UTILS::MATH::IGoomRand& goomRand) noexcept;

  auto Update() noexcept -> void;

  [[nodiscard]] auto GetLerpT() const noexcept -> float;
  [[nodiscard]] auto GetColorIndexes() const noexcept -> const std::array<uint32_t, 3>&;

private:
  [[maybe_unused]]const PluginInfo& m_goomInfo;
  const UTILS::MATH::IGoomRand& m_goomRand;

  std::array<uint32_t, 3> m_colorIndexes{};

  static constexpr uint32_t MIN_NUM_LERP_ON_STEPS     = 10U;
  static constexpr uint32_t MAX_NUM_LERP_ON_STEPS     = 30U;
  static constexpr uint32_t DEFAULT_NUM_LERP_ON_STEPS = MIN_NUM_LERP_ON_STEPS;
  UTILS::TValue m_lerpT{UTILS::TValue::StepType::CONTINUOUS_REVERSIBLE, DEFAULT_NUM_LERP_ON_STEPS};
  float m_currentT = 0.0F;

  static constexpr uint32_t MIN_LERP_OFF_TIME     = 500U;
  static constexpr uint32_t MAX_LERP_OFF_TIME     = 5000U;
  static constexpr uint32_t DEFAULT_LERP_OFF_TIME = MIN_LERP_OFF_TIME;
  UTILS::Timer m_lerpOffTimer{DEFAULT_LERP_OFF_TIME, false};

  [[nodiscard]] auto RestartLerp() const noexcept -> bool;
};

inline auto RgbBgrLerper::GetLerpT() const noexcept -> float
{
  return m_currentT;
}

inline auto RgbBgrLerper::GetColorIndexes() const noexcept -> const std::array<uint32_t, 3>&
{
  return m_colorIndexes;
}

} // namespace GOOM::VISUAL_FX::SHADERS
