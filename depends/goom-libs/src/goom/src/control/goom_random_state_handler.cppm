module;

#include <cstdint>
#include <unordered_set>

export module Goom.Control.GoomRandomStateHandler;

import Goom.Control.GoomDrawables;
import Goom.Control.GoomStateHandler;
import Goom.Utils.Math.GoomRandBase;

using GOOM::UTILS::MATH::IGoomRand;
using GOOM::UTILS::MATH::NumberRange;

export namespace GOOM::CONTROL
{

class GoomRandomStateHandler : public IGoomStateHandler
{
public:
  explicit GoomRandomStateHandler(const IGoomRand& goomRand);

  auto ChangeToNextState() -> void override;

  [[nodiscard]] auto GetCurrentState() const noexcept -> const GoomDrawablesState& override;

private:
  const IGoomRand* m_goomRand;
  GoomDrawablesState m_currentDrawablesState{};
  std::unordered_set<GoomDrawables> m_DrawablesPool = GetFullDrawablesPool();
  [[nodiscard]] static auto GetFullDrawablesPool() -> std::unordered_set<GoomDrawables>;
  [[nodiscard]] auto GetRandomwDrawableFromPool() -> GoomDrawables;


  static constexpr auto DRAWABLES_RANGE = NumberRange{1U, 5U};
  auto MakeNewState(uint32_t numDrawables) noexcept -> void;
  auto AddDrawables(uint32_t numDrawables) noexcept -> void;
  auto RemoveDrawables(uint32_t numDrawables) noexcept -> void;
};

} // namespace GOOM::CONTROL

namespace GOOM::CONTROL
{

inline auto GoomRandomStateHandler::GetCurrentState() const noexcept -> const GoomDrawablesState&
{
  return  m_currentDrawablesState;
}

} // namespace GOOM::CONTROL
