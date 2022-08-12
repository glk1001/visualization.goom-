#pragma once

namespace GOOM::CONTROL
{

static constexpr auto USE_FORCED_GOOM_STATE   = false;
static constexpr auto USE_FORCED_FILTER_MODE  = false;
static constexpr auto USE_FORCED_AFTER_EFFECT = false;

static constexpr auto ALL_AFTER_EFFECTS_TURNED_OFF = false;

static_assert(((not ALL_AFTER_EFFECTS_TURNED_OFF) and (not USE_FORCED_AFTER_EFFECT)) or
              (ALL_AFTER_EFFECTS_TURNED_OFF and (not USE_FORCED_AFTER_EFFECT)) or
              ((not ALL_AFTER_EFFECTS_TURNED_OFF) and USE_FORCED_AFTER_EFFECT));

} // namespace GOOM::CONTROL
