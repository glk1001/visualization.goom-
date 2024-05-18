module;

#include "goom/goom_config.h"

#include <memory>

export module Goom.FilterFx.FilterSettings;

import Goom.FilterFx.AfterEffects.AfterEffectsStates;
import Goom.FilterFx.FilterUtils.GoomLerpData;
import Goom.FilterFx.FilterSpeed;
import Goom.FilterFx.ZoomAdjustmentEffect;
import Goom.Lib.Point2d;

export namespace GOOM::FILTER_FX
{

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

  FilterMultiplierEffectsSettings filterMultiplierEffectsSettings;
  AFTER_EFFECTS::AfterEffectsStates::AfterEffectsSettings afterEffectsSettings;
};

struct FilterSettings
{
  bool filterEffectsSettingsHaveChanged = false;
  FilterEffectsSettings filterEffectsSettings{};
  FILTER_FX::FILTER_UTILS::GoomLerpData transformBufferLerpData{};
};

} // namespace GOOM::FILTER_FX
