#pragma once

#include "goom_graphic.h"
#include "goom_states.h"
#include "utils/goomrand.h"

#include <string>
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

class GoomRandomStates
{
public:
  using DrawablesState = std::unordered_set<GoomDrawables>;

  GoomRandomStates();

  [[nodiscard]] auto GetCurrentState() const -> GoomStates;
  [[nodiscard]] auto GetCurrentDrawables() const -> GoomRandomStates::DrawablesState;

  void DoRandomStateChange();

private:
  static const UTILS::Weights<GoomStates> WEIGHTED_STATES;
  GoomStates m_currentState{};
};

inline auto GoomRandomStates::GetCurrentState() const -> GoomStates
{
  return m_currentState;
}

#if __cplusplus <= 201402L
} // namespace CONTROL
} // namespace GOOM
#else
} // namespace GOOM::CONTROL
#endif
