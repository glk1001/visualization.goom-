#pragma once

#include "goom_graphic.h"
#include "goom_state_handler.h"
#include "goom_states.h"
#include "utils/randutils.h"

#if __cplusplus <= 201402L
namespace GOOM
{
namespace CONTROL
{
#else
namespace GOOM::CONTROL
{
#endif

class GoomRandomStateHandler : public IGoomStateHandler
{
public:
  GoomRandomStateHandler();

  void ChangeToNextState() override;

  [[nodiscard]] auto GetCurrentState() const -> GoomStates override;

private:
  static const UTILS::Weights<GoomStates> WEIGHTED_STATES;
  GoomStates m_currentState{};
};

inline auto GoomRandomStateHandler::GetCurrentState() const -> GoomStates
{
  return m_currentState;
}

#if __cplusplus <= 201402L
} // namespace CONTROL
} // namespace GOOM
#else
} // namespace GOOM::CONTROL
#endif
