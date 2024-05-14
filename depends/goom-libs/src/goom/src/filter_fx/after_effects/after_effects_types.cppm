module;

#include "goom/goom_types.h"

export module Goom.FilterFx.AfterEffects.AfterEffectsTypes;

export namespace GOOM::FILTER_FX::AFTER_EFFECTS
{

enum class AfterEffectsTypes : UnderlyingEnumType
{
  HYPERCOS,
  IMAGE_VELOCITY,
  NOISE,
  PLANES,
  ROTATION,
  TAN_EFFECT,
  XY_LERP_EFFECT,
};

} // namespace GOOM::FILTER_FX::AFTER_EFFECTS
