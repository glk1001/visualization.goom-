module;

#include <vector>

export module Goom.Control.GoomForcedStateHandler;

import Goom.Control.GoomDrawables;
import Goom.Control.GoomStateHandler;

export namespace GOOM::CONTROL
{

class GoomForcedStateHandler : public IGoomStateHandler
{
public:
  GoomForcedStateHandler() noexcept;

  auto ChangeToNextState() -> void override;

  [[nodiscard]] auto GetCurrentState() const noexcept -> const GoomDrawablesState& override;

private:
  GoomDrawablesState m_constDrawablesState;
  [[nodiscard]] static auto GetGoomDrawablesState(
      const std::vector<GoomDrawables>& drawables) noexcept -> GoomDrawablesState;
};

} // namespace GOOM::CONTROL

namespace GOOM::CONTROL
{

inline auto GoomForcedStateHandler::ChangeToNextState() -> void
{
  // No changes allowed.
}

inline auto GoomForcedStateHandler::GetCurrentState() const noexcept -> const GoomDrawablesState&
{
  return m_constDrawablesState;
}

} // namespace GOOM::CONTROL
