#pragma once

#include "after_effects/after_effects_states.h"
#include "filter_speed.h"
#include "normalized_coords.h"
#include "point2d.h"

#include <cstdint>
#include <memory>

namespace GOOM::FILTER_FX
{

class IZoomInCoefficientsEffect;

struct TransformBufferLerpData
{
  float lerpIncrement = 0.0F;
  float lerpToMaxLerp = 0.0F;
};

struct FilterEffectsSettings
{
  Vitesse vitesse;

  float maxZoomInCoeff;
  float baseZoomInCoeffFactorMultiplier;
  float afterEffectsVelocityMultiplier;
  std::shared_ptr<IZoomInCoefficientsEffect> zoomInCoefficientsEffect;

  Point2dInt zoomMidpoint;
  Viewport filterViewport{};

  AFTER_EFFECTS::AfterEffectsStates::AfterEffectsSettings afterEffectsSettings;
};

struct FilterSettings
{
  bool filterEffectsSettingsHaveChanged = false;
  FilterEffectsSettings filterEffectsSettings{};
  TransformBufferLerpData transformBufferLerpData{};
};

} // namespace GOOM::FILTER_FX
