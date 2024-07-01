export module Goom.Control.GoomRandomStateHandler;

import Goom.Control.GoomDrawables;
import Goom.Control.GoomStateHandler;
import Goom.Control.GoomStates;
import Goom.Utils.Math.GoomRandBase;

using GOOM::UTILS::MATH::IGoomRand;
using GOOM::UTILS::MATH::Weights;

export namespace GOOM::CONTROL
{

class GoomRandomStateHandler : public IGoomStateHandler
{
public:
  explicit GoomRandomStateHandler(const IGoomRand& goomRand);

  auto ChangeToNextState() -> void override;

  [[nodiscard]] auto GetCurrentState() const noexcept -> const GoomDrawablesState& override;

private:
  Weights<GoomStates> m_weightedStates;
  GoomStates m_currentState{};
};

} // namespace GOOM::CONTROL

namespace GOOM::CONTROL
{

inline auto GoomRandomStateHandler::GetCurrentState() const noexcept -> const GoomDrawablesState&
{
  return GoomStateInfo::GetDrawablesState(m_currentState);
}

} // namespace GOOM::CONTROL
