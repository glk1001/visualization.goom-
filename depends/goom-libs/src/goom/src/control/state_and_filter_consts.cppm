export module Goom.Control.StateAndFilterConsts;

export namespace GOOM::CONTROL
{

inline constexpr auto USE_FORCED_GOOM_STATE   = true;
inline constexpr auto USE_FORCED_FILTER_MODE  = false;
inline constexpr auto USE_FORCED_AFTER_EFFECT = false;

inline constexpr auto ALL_AFTER_EFFECTS_TURNED_OFF = false;

static_assert(not(ALL_AFTER_EFFECTS_TURNED_OFF and USE_FORCED_AFTER_EFFECT));

} // namespace GOOM::CONTROL
