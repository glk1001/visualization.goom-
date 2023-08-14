#include "goom_state_handler.h"

#include "goom_states.h"

namespace GOOM::CONTROL
{

auto IGoomStateHandler::GetCurrentDrawables() const -> DrawablesState
{
  DrawablesState currentDrawables{};
  for (const auto& drawableInfo : GoomStateInfo::GetStateInfo(GetCurrentState()).drawablesInfo)
  {
    currentDrawables.push_back(drawableInfo.fx);
  }
  return currentDrawables;
}

} // namespace GOOM::CONTROL
