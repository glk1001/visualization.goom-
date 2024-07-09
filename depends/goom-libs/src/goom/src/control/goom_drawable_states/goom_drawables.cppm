module;

#include <bitset>
#include <string>
#include <vector>

export module Goom.Control.GoomDrawables;

import Goom.Utils.EnumUtils;
import Goom.Lib.GoomTypes;

using GOOM::UTILS::EnumMap;
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

// Clang-tidy False negative - EnumMap m_drawablesBuffIntensities has a default constructor.
// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init,hicpp-member-init)
class GoomDrawablesState
{
  using DrawablesBitset = std::bitset<NUM<GoomDrawables>>;

public:
  GoomDrawablesState() noexcept = default;
  GoomDrawablesState(const std::vector<GoomDrawables>& drawables,
                     const std::vector<float>& drawablesBuffIntensitiesVec) noexcept;

  [[nodiscard]] auto GetName() const -> const std::string&;
  [[nodiscard]] auto GetDrawables() const noexcept -> const std::vector<GoomDrawables>&;
  [[nodiscard]] auto GetDrawablesBuffIntensities() const noexcept -> const std::vector<float>&;

  [[nodiscard]] auto GetBuffIntensity(GoomDrawables goomDrawable) const noexcept -> float;

  [[nodiscard]] auto IsMultiThreaded() const noexcept -> bool;

  [[nodiscard]] auto GetId() const noexcept -> DrawablesBitset;
  [[nodiscard]] auto HasSameId(const DrawablesBitset& id2) const noexcept -> bool;

  auto operator==(const GoomDrawablesState& state2) noexcept -> bool;

private:
  std::vector<GoomDrawables> m_drawables;
  std::vector<float> m_drawablesBuffIntensitiesVec;
  EnumMap<GoomDrawables, float> m_drawablesBuffIntensities;
  auto UpdateDrawablesBuffIntensities() noexcept -> void;

  DrawablesBitset m_drawablesAsBitset;
  auto UpdateDrawablesAsBitset() noexcept -> void;

  mutable std::string m_stateName;
};

} // namespace GOOM::CONTROL

namespace GOOM::CONTROL
{

inline auto GoomDrawablesState::operator==(const GoomDrawablesState& state2) noexcept -> bool
{
  return m_drawablesAsBitset == state2.m_drawablesAsBitset;
}

inline auto GoomDrawablesState::GetDrawables() const noexcept -> const std::vector<GoomDrawables>&
{
  return m_drawables;
}

inline auto GoomDrawablesState::GoomDrawablesState::GetId() const noexcept -> DrawablesBitset
{
  return m_drawablesAsBitset;
}

inline auto GoomDrawablesState::HasSameId(const DrawablesBitset& id2) const noexcept -> bool
{
  return m_drawablesAsBitset == id2;
}

inline auto GoomDrawablesState::GetDrawablesBuffIntensities() const noexcept
    -> const std::vector<float>&
{
  return m_drawablesBuffIntensitiesVec;
}

inline auto GoomDrawablesState::GetBuffIntensity(const GoomDrawables goomDrawable) const noexcept
    -> float
{
  return m_drawablesBuffIntensities[goomDrawable];
}

} // namespace GOOM::CONTROL
