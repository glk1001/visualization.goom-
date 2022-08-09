#pragma once

#include "after_effects/after_effects_states.h"
#include "filter_speed.h"
#include "point2d.h"

#include <cstdint>
#include <memory>

namespace GOOM::FILTER_FX
{

class ISpeedCoefficientsEffect;

struct ZoomFilterBufferSettings
{
  int32_t tranLerpIncrement;
  float tranLerpToMaxSwitchMult;
};

struct ZoomFilterEffectsSettings
{
  Vitesse vitesse;

  float maxSpeedCoeff;
  std::shared_ptr<ISpeedCoefficientsEffect> speedCoefficientsEffect;

  Point2dInt zoomMidpoint; // milieu de l'effet

  AFTER_EFFECTS::AfterEffectsSettings afterEffectsSettings;
};

struct ZoomFilterSettings
{
  bool filterEffectsSettingsHaveChanged = false;
  ZoomFilterEffectsSettings filterEffectsSettings{};
  ZoomFilterBufferSettings filterBufferSettings{};
};

} // namespace GOOM::FILTER_FX
