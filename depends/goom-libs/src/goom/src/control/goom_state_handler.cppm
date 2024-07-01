export module Goom.Control.GoomStateHandler;

import Goom.Control.GoomDrawables;

export namespace GOOM::CONTROL
{

class IGoomStateHandler
{
public:
  IGoomStateHandler() noexcept                                   = default;
  IGoomStateHandler(const IGoomStateHandler&) noexcept           = delete;
  IGoomStateHandler(IGoomStateHandler&&) noexcept                = delete;
  virtual ~IGoomStateHandler() noexcept                          = default;
  auto operator=(const IGoomStateHandler&) -> IGoomStateHandler& = delete;
  auto operator=(IGoomStateHandler&&) -> IGoomStateHandler&      = delete;

  virtual void ChangeToNextState() = 0;

  [[nodiscard]] virtual auto GetCurrentState() const noexcept -> const GoomDrawablesState& = 0;
};

} // namespace GOOM::CONTROL
