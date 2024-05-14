module;

#include "goom/goom_config.h"

#include <memory>
#include <string>

module Goom.FilterFx.FilterEffects.ZoomAdjustmentEffectFactory;

import Goom.FilterFx.FilterEffects.AdjustmentEffects.Amulet;
import Goom.FilterFx.FilterEffects.AdjustmentEffects.ComplexRational;
import Goom.FilterFx.FilterEffects.AdjustmentEffects.CrystalBall;
import Goom.FilterFx.FilterEffects.AdjustmentEffects.DistanceField;
import Goom.FilterFx.FilterEffects.AdjustmentEffects.ExpReciprocal;
import Goom.FilterFx.FilterEffects.AdjustmentEffects.ImageZoomAdjustment;
import Goom.FilterFx.FilterEffects.AdjustmentEffects.Mobius;
import Goom.FilterFx.FilterEffects.AdjustmentEffects.Newton;
import Goom.FilterFx.FilterEffects.AdjustmentEffects.PerlinNoise;
import Goom.FilterFx.FilterEffects.AdjustmentEffects.Scrunch;
import Goom.FilterFx.FilterEffects.AdjustmentEffects.Speedway;
import Goom.FilterFx.FilterEffects.AdjustmentEffects.UniformZoomAdjustmentEffect;
import Goom.FilterFx.FilterEffects.AdjustmentEffects.Wave;
import Goom.FilterFx.FilterEffects.AdjustmentEffects.YOnly;
import Goom.FilterFx.FilterSettingsService;
import Goom.FilterFx.ZoomAdjustmentEffect;
import Goom.Utils.Math.GoomRandBase;

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

using GOOM::UTILS::MATH::IGoomRand;

auto CreateZoomAdjustmentEffect(const ZoomFilterMode filterMode,
                                const IGoomRand& goomRand,
                                const std::string& resourcesDirectory)
    -> std::shared_ptr<IZoomAdjustmentEffect>
{
  switch (filterMode)
  {
    case ZoomFilterMode::AMULET_MODE:
      return std::make_shared<Amulet>(goomRand);
    case ZoomFilterMode::COMPLEX_RATIONAL_MODE:
      return std::make_shared<ComplexRational>(goomRand);
    case ZoomFilterMode::CRYSTAL_BALL_MODE0:
      return std::make_shared<CrystalBall>(CrystalBall::Modes::MODE0, goomRand);
    case ZoomFilterMode::CRYSTAL_BALL_MODE1:
      return std::make_shared<CrystalBall>(CrystalBall::Modes::MODE1, goomRand);
    case ZoomFilterMode::DISTANCE_FIELD_MODE0:
      return std::make_shared<DistanceField>(DistanceField::Modes::MODE0, goomRand);
    case ZoomFilterMode::DISTANCE_FIELD_MODE1:
      return std::make_shared<DistanceField>(DistanceField::Modes::MODE1, goomRand);
    case ZoomFilterMode::DISTANCE_FIELD_MODE2:
      return std::make_shared<DistanceField>(DistanceField::Modes::MODE2, goomRand);
    case ZoomFilterMode::EXP_RECIPROCAL_MODE:
      return std::make_shared<ExpReciprocal>(goomRand);
    case ZoomFilterMode::HYPERCOS_MODE0:
    case ZoomFilterMode::HYPERCOS_MODE1:
    case ZoomFilterMode::HYPERCOS_MODE2:
    case ZoomFilterMode::HYPERCOS_MODE3:
      return std::make_shared<UniformZoomAdjustmentEffect>();
    case ZoomFilterMode::IMAGE_DISPLACEMENT_MODE:
      return std::make_shared<ImageZoomAdjustment>(resourcesDirectory, goomRand);
    case ZoomFilterMode::MOBIUS_MODE:
      return std::make_shared<Mobius>(goomRand);
    case ZoomFilterMode::NEWTON_MODE:
      return std::make_shared<Newton>(goomRand);
    case ZoomFilterMode::NORMAL_MODE:
      return std::make_shared<UniformZoomAdjustmentEffect>();
    case ZoomFilterMode::PERLIN_NOISE_MODE:
      return std::make_shared<PerlinNoise>(goomRand);
    case ZoomFilterMode::SCRUNCH_MODE:
      return std::make_shared<Scrunch>(goomRand);
    case ZoomFilterMode::SPEEDWAY_MODE0:
      return std::make_shared<Speedway>(Speedway::Modes::MODE0, goomRand);
    case ZoomFilterMode::SPEEDWAY_MODE1:
      return std::make_shared<Speedway>(Speedway::Modes::MODE1, goomRand);
    case ZoomFilterMode::SPEEDWAY_MODE2:
      return std::make_shared<Speedway>(Speedway::Modes::MODE2, goomRand);
    case ZoomFilterMode::WATER_MODE:
      return std::make_shared<UniformZoomAdjustmentEffect>();
    case ZoomFilterMode::WAVE_SQ_DIST_ANGLE_EFFECT_MODE0:
      return std::make_shared<Wave>(Wave::Modes::SQ_DIST_ANGLE_EFFECT_MODE0, goomRand);
    case ZoomFilterMode::WAVE_SQ_DIST_ANGLE_EFFECT_MODE1:
      return std::make_shared<Wave>(Wave::Modes::SQ_DIST_ANGLE_EFFECT_MODE1, goomRand);
    case ZoomFilterMode::WAVE_ATAN_ANGLE_EFFECT_MODE0:
      return std::make_shared<Wave>(Wave::Modes::ATAN_ANGLE_EFFECT_MODE0, goomRand);
    case ZoomFilterMode::WAVE_ATAN_ANGLE_EFFECT_MODE1:
      return std::make_shared<Wave>(Wave::Modes::ATAN_ANGLE_EFFECT_MODE1, goomRand);
    case ZoomFilterMode::Y_ONLY_MODE:
      return std::make_shared<YOnly>(goomRand);
  }
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
