export module Goom.Control.GoomRandomStateHandler;

import Goom.Control.GoomStateHandler;
import Goom.Control.GoomStates;
import Goom.Utils.Math.GoomRandBase;

export namespace GOOM::CONTROL
{

class GoomRandomStateHandler : public IGoomStateHandler
{
public:
  explicit GoomRandomStateHandler(const UTILS::MATH::IGoomRand& goomRand);

  void ChangeToNextState() override;

  [[nodiscard]] auto GetCurrentState() const -> GoomStates override;

private:
  UTILS::MATH::Weights<GoomStates> m_weightedStates;
  GoomStates m_currentState{};
};

} // namespace GOOM::CONTROL

namespace GOOM::CONTROL
{

inline auto GoomRandomStateHandler::GetCurrentState() const -> GoomStates
{
  return m_currentState;
}

} // namespace GOOM::CONTROL
