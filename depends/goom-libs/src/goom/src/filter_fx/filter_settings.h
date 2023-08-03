#pragma once

#include "after_effects/after_effects_states.h"
#include "filter_speed.h"
#include "normalized_coords.h"
#include "point2d.h"

#include <cstdint>
#include <memory>

namespace GOOM::FILTER_FX
{

class IZoomAdjustmentEffect;

struct TransformBufferLerpData
{
  float lerpIncrement = 0.0F;
  float lerpToMaxLerp = 0.0F;
};

struct FilterMultiplierEffectsSettings
{
  bool isActive                    = false;
  float xFreq                      = 1.0F;
  float yFreq                      = 1.0F;
  float xAmplitude                 = 1.0F;
  float yAmplitude                 = 1.0F;
  float lerpZoomAdjustmentToCoords = 1.0F;
};

struct FilterEffectsSettings
{
  Vitesse vitesse;

  float maxZoomAdjustment;
  float baseZoomAdjustmentFactorMultiplier;
  float afterEffectsVelocityMultiplier;
  std::shared_ptr<IZoomAdjustmentEffect> zoomAdjustmentEffect;

  Point2dInt zoomMidpoint;
  Viewport filterViewport{};

  FilterMultiplierEffectsSettings filterMultiplierEffectsSettings;
  AFTER_EFFECTS::AfterEffectsStates::AfterEffectsSettings afterEffectsSettings;
};

struct FilterSettings
{
  bool filterEffectsSettingsHaveChanged = false;
  FilterEffectsSettings filterEffectsSettings{};
  TransformBufferLerpData transformBufferLerpData{};
};

} // namespace GOOM::FILTER_FX
