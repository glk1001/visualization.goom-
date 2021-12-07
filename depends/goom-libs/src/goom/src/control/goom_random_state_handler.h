#pragma once

#include "goom_graphic.h"
#include "goom_state_handler.h"
#include "goom_states.h"
#include "utils/goom_rand_base.h"

namespace GOOM::CONTROL
{

class GoomRandomStateHandler : public IGoomStateHandler
{
public:
  explicit GoomRandomStateHandler(const UTILS::IGoomRand& goomRand);

  void ChangeToNextState() override;

  [[nodiscard]] auto GetCurrentState() const -> GoomStates override;

private:
  const UTILS::Weights<GoomStates> m_weightedStates;
  GoomStates m_currentState{};
};

inline auto GoomRandomStateHandler::GetCurrentState() const -> GoomStates
{
  return m_currentState;
}

} // namespace GOOM::CONTROL
