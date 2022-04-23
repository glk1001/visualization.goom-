#pragma once

#include "goomutils/enumutils.h"
#include "goomutils/goomrand.h"
#include "visual_fx/lines_fx.h"

#include <array>
#include <cstdint>

#if __cplusplus <= 201402L
namespace GOOM
{
namespace CONTROL
{
#else
namespace GOOM::CONTROL
{
#endif

class GoomEvents
{
public:
  GoomEvents() noexcept;
  GoomEvents(const GoomEvents&) noexcept = delete;
  GoomEvents(GoomEvents&&) noexcept = delete;
  ~GoomEvents() = default;
  auto operator=(const GoomEvents&) -> GoomEvents& = delete;
  auto operator=(GoomEvents&&) -> GoomEvents& = delete;

  enum class GoomEvent
  {
    CHANGE_FILTER_MODE = 0,
    CHANGE_STATE,
    CHANGE_TO_MEGA_LENT_MODE,
    CHANGE_LINE_CIRCLE_AMPLITUDE,
    CHANGE_LINE_CIRCLE_PARAMS,
    CHANGE_H_LINE_PARAMS,
    CHANGE_V_LINE_PARAMS,
    REDUCE_LINE_MODE,
    UPDATE_LINE_MODE,
    CHANGE_LINE_TO_BLACK,
    CHANGE_GOOM_LINE,
    FILTER_REVERSE_ON,
    FILTER_REVERSE_OFF_AND_STOP_SPEED,
    FILTER_VITESSE_STOP_SPEED_MINUS1,
    FILTER_VITESSE_STOP_SPEED,
    FILTER_CHANGE_VITESSE_AND_TOGGLE_REVERSE,
    TURN_OFF_NOISE,
    FILTER_TOGGLE_ROTATION,
    FILTER_INCREASE_ROTATION,
    FILTER_DECREASE_ROTATION,
    FILTER_STOP_ROTATION,
    CHANGE_BLOCKY_WAVY_TO_ON,
    CHANGE_ZOOM_FILTER_ALLOW_OVEREXPOSED_TO_ON,
    _NUM // unused and must be last
  };

  auto Happens(GoomEvent event) const -> bool;
  auto GetRandomLineTypeEvent() const -> VISUAL_FX::LinesFx::LineType;

private:
  static constexpr size_t NUM_GOOM_EVENTS = UTILS::NUM<GoomEvent>;
  struct WeightedEvent
  {
    GoomEvent event;
    // m out of n
    uint32_t m;
    uint32_t outOf;
  };
  const std::array<WeightedEvent, NUM_GOOM_EVENTS> m_weightedEvents;
  const std::array<std::pair<VISUAL_FX::LinesFx::LineType, size_t>,
                   VISUAL_FX::LinesFx::NUM_LINE_TYPES>
      m_weightedLineEvents;
  const UTILS::Weights<VISUAL_FX::LinesFx::LineType> m_lineTypeWeights;
};

inline auto GoomEvents::GetRandomLineTypeEvent() const -> VISUAL_FX::LinesFx::LineType
{
  return m_lineTypeWeights.GetRandomWeighted();
}

#if __cplusplus <= 201402L
} // namespace CONTROL
} // namespace GOOM
#else
} // namespace GOOM::CONTROL
#endif
