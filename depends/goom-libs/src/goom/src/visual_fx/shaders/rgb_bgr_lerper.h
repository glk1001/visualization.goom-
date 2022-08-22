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
  [[nodiscard]] auto GetSrceColorIndexes() const noexcept -> const std::array<uint32_t, 3>&;
  [[nodiscard]] auto GetDestColorIndexes() const noexcept -> const std::array<uint32_t, 3>&;

private:
  const PluginInfo& m_goomInfo;
  const UTILS::MATH::IGoomRand& m_goomRand;

  std::array<uint32_t, 3> m_srceColorIndexes{};
  std::array<uint32_t, 3> m_destColorIndexes{};

  static constexpr uint32_t MIN_NUM_LERP_ON_STEPS     = 10U;
  static constexpr uint32_t MAX_NUM_LERP_ON_STEPS     = 30U;
  static constexpr uint32_t DEFAULT_NUM_LERP_ON_STEPS = MIN_NUM_LERP_ON_STEPS;
  UTILS::TValue m_lerpT{UTILS::TValue::StepType::SINGLE_CYCLE, DEFAULT_NUM_LERP_ON_STEPS};

  static constexpr uint32_t MIN_LERP_OFF_TIME     = 500U;
  static constexpr uint32_t MAX_LERP_OFF_TIME     = 5000U;
  static constexpr uint32_t DEFAULT_LERP_OFF_TIME = MIN_LERP_OFF_TIME;
  UTILS::Timer m_lerpOffTimer{DEFAULT_LERP_OFF_TIME, false};

  [[nodiscard]] auto RestartLerp() const noexcept -> bool;
};

inline auto RgbBgrLerper::GetLerpT() const noexcept -> float
{
  return m_lerpT();
}

inline auto RgbBgrLerper::GetSrceColorIndexes() const noexcept -> const std::array<uint32_t, 3>&
{
  return m_srceColorIndexes;
}

inline auto RgbBgrLerper::GetDestColorIndexes() const noexcept -> const std::array<uint32_t, 3>&
{
  return m_destColorIndexes;
}

} // namespace GOOM::VISUAL_FX::SHADERS
