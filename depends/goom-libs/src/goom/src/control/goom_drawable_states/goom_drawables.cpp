module;

#include <algorithm>
#include <bitset>
#include <cstdint>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

module Goom.Control.GoomDrawables;

import Goom.Control.GoomDrawablesData;
import Goom.Lib.AssertUtils;

namespace GOOM::CONTROL
{

GoomDrawablesState::GoomDrawablesState(
    const std::vector<GoomDrawables>& drawables,
    const std::vector<float>& drawablesBuffIntensitiesVec) noexcept
  : m_drawables{drawables}, m_drawablesBuffIntensitiesVec{drawablesBuffIntensitiesVec}
{
  Expects(drawables.size() == drawablesBuffIntensitiesVec.size());

  UpdateDrawablesAsBitset();
  UpdateDrawablesBuffIntensities();

  Ensures(drawables.size() == m_drawablesAsBitset.count());
}

auto GoomDrawablesState::UpdateDrawablesBuffIntensities() noexcept -> void
{
  for (auto i = 0U; i < m_drawables.size(); ++i)
  {
    m_drawablesBuffIntensities[m_drawables[i]] = m_drawablesBuffIntensitiesVec[i];
  }
}

auto GoomDrawablesState::UpdateDrawablesAsBitset() noexcept -> void
{
  m_drawablesAsBitset.reset();

  for (auto drawable : m_drawables)
  {
    m_drawablesAsBitset[static_cast<uint32_t>(drawable)] = true;
  }
}

auto GoomDrawablesState::GetName() const -> const std::string&
{
  if (m_stateName.empty())
  {
    m_stateName = GetDrawablesStateName();
  }
  return m_stateName;
}

auto GoomDrawablesState::GetDrawablesStateName() const -> std::string
{
  static constexpr auto DELIM = std::string_view{"_"};

  return std::ranges::to<std::string>(
      m_drawables |
      std::views::transform([](const auto drawable) { return DRAWABLE_NAMES[drawable]; }) |
      std::views::join_with(DELIM));
}

auto GoomDrawablesState::IsMultiThreaded() const noexcept -> bool
{
  return std::ranges::any_of(m_drawables,
                             [](const auto& drawable) { return STATE_MULTI_THREADED[drawable]; });
}

} // namespace GOOM::CONTROL
