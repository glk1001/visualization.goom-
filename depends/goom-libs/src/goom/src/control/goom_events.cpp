#include "goom_events.h"

#include "utils/math/goom_rand_base.h"

namespace GOOM::CONTROL
{

using UTILS::MATH::IGoomRand;

// clang-format off
static constexpr float CHANGE_FILTER_MODE_PROB                       = 0.05F;
static constexpr float CHANGE_STATE_PROB                             = 0.50F;
static constexpr float CHANGE_TO_MEGA_LENT_MODE_PROB                 = 1.0F / 700.F;
static constexpr float FILTER_REVERSE_ON_PROB                        = 0.10F;
static constexpr float FILTER_REVERSE_OFF_AND_STOP_SPEED_PROB        = 0.20F;
static constexpr float FILTER_VITESSE_STOP_SPEED_MINUS1_PROB         = 0.20F;
static constexpr float FILTER_VITESSE_STOP_SPEED_PROB                = 0.10F;
static constexpr float FILTER_CHANGE_VITESSE_AND_TOGGLE_REVERSE_PROB = 0.05F;
static constexpr float FILTER_TOGGLE_ROTATION_PROB                   = 0.125F;
static constexpr float FILTER_INCREASE_ROTATION_PROB                 = 0.25F;
static constexpr float FILTER_DECREASE_ROTATION_PROB                 = 0.875F;
static constexpr float FILTER_STOP_ROTATION_PROB                     = 0.25F;
// clang-format on

GoomEvents::GoomEvents(const IGoomRand& goomRand)
  : m_goomRand{goomRand},
    // clang-format off
    m_weightedEvents{{
        { GoomEvent::CHANGE_FILTER_MODE,                       CHANGE_FILTER_MODE_PROB},
        { GoomEvent::CHANGE_STATE,                             CHANGE_STATE_PROB},
        { GoomEvent::CHANGE_TO_MEGA_LENT_MODE,                 CHANGE_TO_MEGA_LENT_MODE_PROB},
        { GoomEvent::FILTER_REVERSE_ON,                        FILTER_REVERSE_ON_PROB},
        { GoomEvent::FILTER_REVERSE_OFF_AND_STOP_SPEED,        FILTER_REVERSE_OFF_AND_STOP_SPEED_PROB},
        { GoomEvent::FILTER_VITESSE_STOP_SPEED_MINUS1,         FILTER_VITESSE_STOP_SPEED_MINUS1_PROB},
        { GoomEvent::FILTER_VITESSE_STOP_SPEED,                FILTER_VITESSE_STOP_SPEED_PROB},
        { GoomEvent::FILTER_CHANGE_VITESSE_AND_TOGGLE_REVERSE, FILTER_CHANGE_VITESSE_AND_TOGGLE_REVERSE_PROB},
        { GoomEvent::FILTER_TOGGLE_ROTATION,                   FILTER_TOGGLE_ROTATION_PROB},
        { GoomEvent::FILTER_INCREASE_ROTATION,                 FILTER_INCREASE_ROTATION_PROB},
        { GoomEvent::FILTER_DECREASE_ROTATION,                 FILTER_DECREASE_ROTATION_PROB},
        { GoomEvent::FILTER_STOP_ROTATION,                     FILTER_STOP_ROTATION_PROB},
    }}
// clang-format on
{
}

auto GoomEvents::Happens(const GoomEvent event) const -> bool
{
  const WeightedEvent& weightedEvent = m_weightedEvents.at(static_cast<size_t>(event));
  return m_goomRand.ProbabilityOf(weightedEvent.probability);
}

} // namespace GOOM::CONTROL
