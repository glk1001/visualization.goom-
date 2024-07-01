module;

#include <algorithm>
#include <print>
#include <string>
#include <vector>

export module Goom.Control.GoomDrawables;

import Goom.Utils.EnumUtils;
import Goom.Utils.Math.GoomRandBase;
import Goom.Lib.AssertUtils;
import Goom.Lib.GoomTypes;

export namespace GOOM::CONTROL
{

using BuffIntensityRange = UTILS::MATH::NumberRange<float>;

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
  GoomDrawablesState(const std::string& stateName,
                     const std::vector<GoomDrawables>& drawablesFx,
                     const std::vector<BuffIntensityRange>& drawablesBuffIntensityRanges) noexcept;

  [[nodiscard]] auto GetName() const noexcept -> const std::string&;
  [[nodiscard]] auto GetDrawables() const noexcept -> const std::vector<GoomDrawables>&;

  [[nodiscard]] auto GetBuffIntensityRange(GoomDrawables goomDrawable) const noexcept
      -> BuffIntensityRange;

  [[nodiscard]] auto IsMultiThreaded() const noexcept -> bool;

private:
  std::string m_stateName;
  std::vector<GoomDrawables> m_drawables;
  std::vector<BuffIntensityRange> m_drawablesBuffIntensityRanges;
};

[[nodiscard]] auto operator==(const GoomDrawablesState& state1,
                              const GoomDrawablesState& state2) noexcept -> bool;

[[nodiscard]] auto IsMultiThreaded(GoomDrawablesState goomDrawablesState) -> bool;

} // namespace GOOM::CONTROL

namespace GOOM::CONTROL
{

inline GoomDrawablesState::GoomDrawablesState(
    const std::string& stateName,
    const std::vector<GoomDrawables>& drawablesFx,
    const std::vector<BuffIntensityRange>& drawablesBuffIntensityRanges) noexcept
  : m_stateName{stateName},
    m_drawables{drawablesFx},
    m_drawablesBuffIntensityRanges{drawablesBuffIntensityRanges}
{
  Expects(drawablesFx.size() == drawablesBuffIntensityRanges.size());
}

inline auto operator==(const GoomDrawablesState& state1,
                       const GoomDrawablesState& state2) noexcept -> bool
{
  return state1.GetName() == state2.GetName();
}

inline auto GoomDrawablesState::GetName() const noexcept -> const std::string&
{
  return m_stateName;
}

inline auto GoomDrawablesState::GetDrawables() const noexcept -> const std::vector<GoomDrawables>&
{
  return m_drawables;
}

} // namespace GOOM::CONTROL

module :private;

namespace GOOM::CONTROL
{

auto GoomDrawablesState::GetBuffIntensityRange(GoomDrawables goomDrawable) const noexcept
    -> BuffIntensityRange
{
  for (auto i = 0U; i < m_drawables.size(); ++i)
  {
    if (m_drawables.at(i) == goomDrawable)
    {
      return m_drawablesBuffIntensityRanges.at(i);
    }
  }

  FailFast();
}

auto GoomDrawablesState::IsMultiThreaded() const noexcept -> bool
{
  static constexpr auto STATE_MULTI_THREADED = UTILS::EnumMap<GoomDrawables, bool>{{{
      {GoomDrawables::CIRCLES, false},
      {GoomDrawables::DOTS, false},
      {GoomDrawables::IFS, false},
      {GoomDrawables::L_SYSTEM, false},
      {GoomDrawables::LINES, false},
      {GoomDrawables::IMAGE, true},
      {GoomDrawables::PARTICLES, false},
      {GoomDrawables::RAINDROPS, false},
      {GoomDrawables::SHAPES, false},
      {GoomDrawables::STARS, false},
      {GoomDrawables::TENTACLES, false},
      {GoomDrawables::TUBES, false},
  }}};

  return std::ranges::any_of(m_drawables,
                             [](const auto& drawable) { return STATE_MULTI_THREADED[drawable]; });
}

} // namespace GOOM::CONTROL
