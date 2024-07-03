module;

#include <algorithm>
#include <bitset>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

module Goom.Control.GoomDrawables;

import Goom.Utils.EnumUtils;
import Goom.Lib.AssertUtils;
import Goom.Lib.GoomTypes;

namespace GOOM::CONTROL
{

using UTILS::EnumMap;

GoomDrawablesState::GoomDrawablesState(const std::vector<GoomDrawables>& drawables,
                                       const std::vector<float>& drawablesBuffIntensities) noexcept
  : m_drawables{drawables},
    m_drawablesBuffIntensities{drawablesBuffIntensities},
    m_drawablesAsBitset{GetDrawablesAsBitset(drawables)}
{
  Expects(drawables.size() == drawablesBuffIntensities.size());
  Ensures(drawables.size() == m_drawablesAsBitset.count());
}

auto GoomDrawablesState::GetName() const noexcept -> const std::string&
{
  if (m_stateName.empty())
  {
    m_stateName = GetDrawablesStateName(m_drawables);
  }
  return m_stateName;
}

auto GoomDrawablesState::GetDrawablesAsBitset(const std::vector<GoomDrawables>& drawables) noexcept
    -> std::bitset<NUM<GoomDrawables>>
{
  auto drawablesAsBitset = std::bitset<NUM<GoomDrawables>>{};

  for (auto drawable : drawables)
  {
    drawablesAsBitset[static_cast<uint32_t>(drawable)] = true;
  }

  return drawablesAsBitset;
}

auto GoomDrawablesState::GetDrawablesStateName(const std::vector<GoomDrawables>& drawables) noexcept
    -> std::string
{
  static constexpr auto DRAWABLE_NAMES = EnumMap<GoomDrawables, std::string_view>{{{
      {GoomDrawables::CIRCLES, "CIRCLS"},
      {GoomDrawables::DOTS, "DOTS"},
      {GoomDrawables::IFS, "IFS"},
      {GoomDrawables::L_SYSTEM, "LSYS"},
      {GoomDrawables::LINES, "LNS"},
      {GoomDrawables::IMAGE, "IMG"},
      {GoomDrawables::PARTICLES, "PART"},
      {GoomDrawables::RAINDROPS, "DROPS"},
      {GoomDrawables::SHAPES, "SHPS"},
      {GoomDrawables::STARS, "STARS"},
      {GoomDrawables::TENTACLES, "TENTCL"},
      {GoomDrawables::TUBES, "TUBES"},
  }}};

  auto stateName = std::string{};

  for (auto i = 0U; i < drawables.size(); ++i)
  {
    stateName += std::string{DRAWABLE_NAMES[drawables[i]]} + "_";
  }
  stateName.pop_back();

  return stateName;
}

auto GoomDrawablesState::GetBuffIntensity(GoomDrawables goomDrawable) const noexcept -> float
{
  for (auto i = 0U; i < m_drawables.size(); ++i)
  {
    if (m_drawables.at(i) == goomDrawable)
    {
      return m_drawablesBuffIntensities.at(i);
    }
  }

  FailFast();
}

auto GoomDrawablesState::IsMultiThreaded() const noexcept -> bool
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

  return std::ranges::any_of(m_drawables,
                             [](const auto& drawable) { return STATE_MULTI_THREADED[drawable]; });
}

} // namespace GOOM::CONTROL
