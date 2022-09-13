#pragma once

#include "after_effects/after_effects_states.h"
#include "filter_speed.h"
#include "point2d.h"

#include <cstdint>
#include <memory>

namespace GOOM::FILTER_FX
{

class IZoomInCoefficientsEffect;

struct ZoomFilterBufferSettings
{
  int32_t tranLerpIncrement;
  float tranLerpToMaxSwitchMult;
};

struct ZoomFilterEffectsSettings
{
  Vitesse vitesse;

  float maxZoomCoeff;
  std::shared_ptr<IZoomInCoefficientsEffect> zoomInCoefficientsEffect;

  Point2dInt zoomMidpoint; // milieu de l'effet

  AFTER_EFFECTS::AfterEffectsStates::AfterEffectsSettings afterEffectsSettings;
};

struct ZoomFilterSettings
{
  bool filterEffectsSettingsHaveChanged = false;
  ZoomFilterEffectsSettings filterEffectsSettings{};
  ZoomFilterBufferSettings filterBufferSettings{};
};

} // namespace GOOM::FILTER_FX
