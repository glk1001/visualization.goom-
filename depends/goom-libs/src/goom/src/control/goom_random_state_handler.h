#pragma once

#include "goom_graphic.h"
#include "goom_state_handler.h"
#include "goom_states.h"
#include "utils/goom_rand_base.h"

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

#if __cplusplus <= 201402L
} // namespace CONTROL
} // namespace GOOM
#else
} // namespace GOOM::CONTROL
#endif
