#pragma once

#include "goom_states.h"

#include <unordered_set>

#if __cplusplus <= 201402L
namespace GOOM
{
namespace CONTROL
{
#else
namespace GOOM::CONTROL
{
#endif

class IGoomStateHandler
{
public:
  using DrawablesState = std::unordered_set<GoomDrawables>;

  IGoomStateHandler() noexcept = default;
  virtual ~IGoomStateHandler() noexcept = default;

  virtual void ChangeToNextState() = 0;

  [[nodiscard]] virtual auto GetCurrentState() const -> GoomStates = 0;
  [[nodiscard]] auto GetCurrentDrawables() const -> DrawablesState;
};

#if __cplusplus <= 201402L
} // namespace CONTROL
} // namespace GOOM
#else
} // namespace GOOM::CONTROL
#endif
