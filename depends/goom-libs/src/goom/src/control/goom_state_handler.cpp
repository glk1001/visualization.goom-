module;

#include <vector>

module Goom.Control.GoomStateHandler;

import Goom.Control.GoomStates;

namespace GOOM::CONTROL
{

auto IGoomStateHandler::GetCurrentDrawables() const -> DrawablesState
{
  DrawablesState currentDrawables{};
  for (const auto drawablesInfo = GoomStateInfo::GetStateInfo(GetCurrentState()).drawablesInfo;
       const auto& drawableInfo : drawablesInfo)
  {
    currentDrawables.push_back(drawableInfo.fx);
  }
  return currentDrawables;
}

} // namespace GOOM::CONTROL
