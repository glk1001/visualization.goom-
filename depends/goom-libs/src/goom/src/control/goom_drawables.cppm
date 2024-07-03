module;

#include <bitset>
#include <string>
#include <vector>

export module Goom.Control.GoomDrawables;

import Goom.Utils.EnumUtils;
import Goom.Lib.GoomTypes;

using GOOM::UTILS::NUM;

export namespace GOOM::CONTROL
{

enum class GoomDrawables : UnderlyingEnumType
{
  CIRCLES = 0,
  DOTS,
  IFS,
  IMAGE,
  L_SYSTEM,
  LINES,
  PARTICLES,
  RAINDROPS,
  SHAPES,
  STARS,
  TENTACLES,
  TUBES,
};

class GoomDrawablesState
{
public:
  GoomDrawablesState() noexcept = default;
  GoomDrawablesState(const std::vector<GoomDrawables>& drawables,
                     const std::vector<float>& drawablesBuffIntensities) noexcept;

  [[nodiscard]] auto GetName() const noexcept -> const std::string&;
  [[nodiscard]] auto GetDrawables() const noexcept -> const std::vector<GoomDrawables>&;
  [[nodiscard]] auto GetDrawablesBuffIntensities() const noexcept -> const std::vector<float>&;

  [[nodiscard]] auto GetBuffIntensity(GoomDrawables goomDrawable) const noexcept -> float;

  [[nodiscard]] auto IsMultiThreaded() const noexcept -> bool;

private:
  std::vector<GoomDrawables> m_drawables;
  std::vector<float> m_drawablesBuffIntensities;

  std::bitset<NUM<GoomDrawables>> m_drawablesAsBitset;
  [[nodiscard]] static auto GetDrawablesAsBitset(
      const std::vector<GoomDrawables>& drawables) noexcept -> std::bitset<NUM<GoomDrawables>>;
  friend auto operator==(const GoomDrawablesState& state1,
                         const GoomDrawablesState& state2) noexcept -> bool;

  mutable std::string m_stateName;
  [[nodiscard]] static auto GetDrawablesStateName(
      const std::vector<GoomDrawables>& drawables) noexcept -> std::string;
};

[[nodiscard]] auto operator==(const GoomDrawablesState& state1,
                              const GoomDrawablesState& state2) noexcept -> bool;

} // namespace GOOM::CONTROL

namespace GOOM::CONTROL
{

inline auto operator==(const GoomDrawablesState& state1,
                       const GoomDrawablesState& state2) noexcept -> bool
{
  return state1.m_drawablesAsBitset == state2.m_drawablesAsBitset;
}

inline auto GoomDrawablesState::GetDrawables() const noexcept -> const std::vector<GoomDrawables>&
{
  return m_drawables;
}

inline auto GoomDrawablesState::GetDrawablesBuffIntensities() const noexcept
    -> const std::vector<float>&
{
  return m_drawablesBuffIntensities;
}

} // namespace GOOM::CONTROL
