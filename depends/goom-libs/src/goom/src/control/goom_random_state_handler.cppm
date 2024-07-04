module;

#include <cstdint>
#include <vector>

export module Goom.Control.GoomRandomStateHandler;

import Goom.Control.GoomDrawables;
import Goom.Control.GoomStateHandler;
import Goom.Utils.EnumUtils;
import Goom.Utils.Math.GoomRandBase;
import Goom.Lib.GoomTypes;

using GOOM::UTILS::NUM;
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
  const IGoomRand* m_goomRand;

  enum class ChangeType : UnderlyingEnumType
  {
    FRESH_STATE,
    NON_REPEAT_STATE,
    ADD_EXTRA_DRAWABLE,
    ADD_REMOVE_DRAWABLE,
    REMOVE_DRAWABLE,
  };
  Weights<ChangeType> m_weightedChangeTypes;
  auto ChangeToFreshState() -> void;
  auto ChangeToNonRepeatState() -> void;
  auto AddExtraDrawableToCurrentState() -> void;
  auto AddRemoveDrawableToCurrentState() -> void;
  auto RemoveDrawableFromCurrentState() -> void;
  auto ChangeToNewState(uint32_t numRandomDrawables) -> void;

  GoomDrawablesState m_currentDrawablesState{};
  [[nodiscard]] auto GetRandomDrawablesFromPool(uint32_t numDrawables)
      -> std::vector<GoomDrawables>;
  [[nodiscard]] auto GetNextRandomDrawables(uint32_t numDrawables) -> std::vector<GoomDrawables>;
  [[nodiscard]] auto GetBuffIntensities(const std::vector<GoomDrawables>& drawables) const noexcept
      -> std::vector<float>;

  std::vector<GoomDrawables> m_drawablesPool;
  [[nodiscard]] static auto GetFullDrawablesPool(const IGoomRand& goomRand)
      -> std::vector<GoomDrawables>;

  enum class NumDrawables : UnderlyingEnumType
  {
    ONE,
    TWO,
    THREE,
    FOUR,
    FIVE,
  };
  static constexpr auto MAX_NUM_DRAWABLES = NUM<NumDrawables>;
  static_assert(MAX_NUM_DRAWABLES < NUM<GoomDrawables>);
  Weights<NumDrawables> m_weightedNumDrawables{};
  [[nodiscard]] auto GetNextNumDrawables() const -> uint32_t;
  auto MakeNewState(uint32_t numDrawables) noexcept -> void;
  auto AddDrawables(uint32_t numDrawables) noexcept -> void;
  auto RemoveDrawables(uint32_t numDrawables) noexcept -> void;
};

} // namespace GOOM::CONTROL

namespace GOOM::CONTROL
{

inline auto GoomRandomStateHandler::GetCurrentState() const noexcept -> const GoomDrawablesState&
{
  return m_currentDrawablesState;
}

} // namespace GOOM::CONTROL
