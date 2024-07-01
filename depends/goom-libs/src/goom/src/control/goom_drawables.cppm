module;

#include <algorithm>
#include <string_view>
#include <vector>

export module Goom.Control.GoomDrawables;

import Goom.Utils.EnumUtils;
import Goom.Utils.Math.GoomRandBase;
import Goom.Lib.GoomTypes;

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

using BuffIntensityRange = UTILS::MATH::NumberRange<float>;

struct DrawableInfo
{
  GoomDrawables fx{};
  BuffIntensityRange buffIntensityRange{};
};
struct GoomDrawablesState
{
  std::string_view name;
  std::vector<DrawableInfo> drawablesInfo;
};
[[nodiscard]] auto operator==(const GoomDrawablesState& state1,
                              const GoomDrawablesState& state2) noexcept -> bool;

[[nodiscard]] auto IsMultiThreaded(GoomDrawablesState goomDrawablesState) -> bool;

} // namespace GOOM::CONTROL

namespace GOOM::CONTROL
{

auto operator==(const GoomDrawablesState& state1, const GoomDrawablesState& state2) noexcept -> bool
{
  return state1.name == state2.name;
}

} // namespace GOOM::CONTROL

module :private;

namespace GOOM::CONTROL
{

using UTILS::EnumMap;

auto IsMultiThreaded(const GoomDrawablesState goomDrawablesState) -> bool
{

  static constexpr auto STATE_MULTI_THREADED = EnumMap<GoomDrawables, bool>{{{
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

  return std::ranges::any_of(goomDrawablesState.drawablesInfo,
                             [](const auto& goomDrawable)
                             { return STATE_MULTI_THREADED[goomDrawable.fx]; });
}

} // namespace GOOM::CONTROL
