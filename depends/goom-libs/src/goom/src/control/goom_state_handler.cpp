#include "goom_state_handler.h"

#if __cplusplus <= 201402L
namespace GOOM
{
namespace CONTROL
{
#else
namespace GOOM::CONTROL
{
#endif

auto IGoomStateHandler::GetCurrentDrawables() const -> DrawablesState
{
  DrawablesState currentDrawables{};
  for (const auto drawableInfo : GoomStateInfo::GetStateInfo(GetCurrentState()).drawablesInfo)
  {
    currentDrawables.insert(drawableInfo.fx);
  }
  return currentDrawables;
}

#if __cplusplus <= 201402L
} // namespace CONTROL
} // namespace GOOM
#else
} // namespace GOOM::CONTROL
#endif
