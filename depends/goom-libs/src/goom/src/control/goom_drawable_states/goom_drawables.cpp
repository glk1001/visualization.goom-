module;

#include <cstdint>
#include <string>
#include <vector>

module Goom.Control.GoomDrawables;

import Goom.Control.GoomDrawablesData;
import Goom.Lib.AssertUtils;

namespace GOOM::CONTROL
{

// Clang-tidy False negative - EnumMap m_drawablesBuffIntensities has a default constructor.
// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init,hicpp-member-init)
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
    m_stateName = GetDrawablesStateName(m_drawables);
  }
  return m_stateName;
}

auto GoomDrawablesState::IsMultiThreaded() const noexcept -> bool
{
  return AreAnyMultiThreaded(m_drawables);
}

} // namespace GOOM::CONTROL
