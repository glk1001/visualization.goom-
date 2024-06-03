module;

#include "goom/goom_config.h"

#include <format>
#include <memory>
#include <string>

module Goom.FilterFx.FilterEffects.ZoomAdjustmentEffectFactory;

import Goom.FilterFx.FilterEffects.AdjustmentEffects.Amulet;
import Goom.FilterFx.FilterEffects.AdjustmentEffects.ComplexRational;
import Goom.FilterFx.FilterEffects.AdjustmentEffects.CrystalBall;
import Goom.FilterFx.FilterEffects.AdjustmentEffects.DistanceField;
import Goom.FilterFx.FilterEffects.AdjustmentEffects.ExpReciprocal;
import Goom.FilterFx.FilterEffects.AdjustmentEffects.FunctionOfFunction;
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
import Goom.Utils.ArrayUtils;
import Goom.Utils.EnumUtils;

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

using UTILS::Contains;
using UTILS::NUM;
using UTILS::MATH::IGoomRand;

namespace
{

constexpr auto NON_FUNC_OF_FUNC_EFFECTS = std::array{
    ZoomFilterMode::IMAGE_DISPLACEMENT_OF_WAVE_SQ_DIST_ANGLE_MODE0,
    ZoomFilterMode::PERLIN_NOISE_OF_WAVE_SQ_DIST_ANGLE_MODE0,
};

auto CreateFuncZoomAdjustmentEffect(const ZoomFilterMode filterMode,
                                    const IGoomRand& goomRand,
                                    const std::string& resourcesDirectory)
    -> std::unique_ptr<IZoomAdjustmentEffect>
{
  switch (filterMode)
  {
    case ZoomFilterMode::AMULET_MODE:
      return std::make_unique<Amulet>(goomRand);
    case ZoomFilterMode::COMPLEX_RATIONAL_MODE:
      return std::make_unique<ComplexRational>(goomRand);
    case ZoomFilterMode::CRYSTAL_BALL_MODE0:
      return std::make_unique<CrystalBall>(CrystalBall::Modes::MODE0, goomRand);
    case ZoomFilterMode::CRYSTAL_BALL_MODE1:
      return std::make_unique<CrystalBall>(CrystalBall::Modes::MODE1, goomRand);
    case ZoomFilterMode::DISTANCE_FIELD_MODE0:
      return std::make_unique<DistanceField>(DistanceField::Modes::MODE0, goomRand);
    case ZoomFilterMode::DISTANCE_FIELD_MODE1:
      return std::make_unique<DistanceField>(DistanceField::Modes::MODE1, goomRand);
    case ZoomFilterMode::DISTANCE_FIELD_MODE2:
      return std::make_unique<DistanceField>(DistanceField::Modes::MODE2, goomRand);
    case ZoomFilterMode::EXP_RECIPROCAL_MODE:
      return std::make_unique<ExpReciprocal>(goomRand);
    case ZoomFilterMode::HYPERCOS_MODE0:
    case ZoomFilterMode::HYPERCOS_MODE1:
    case ZoomFilterMode::HYPERCOS_MODE2:
    case ZoomFilterMode::HYPERCOS_MODE3:
      return std::make_unique<UniformZoomAdjustmentEffect>();
    case ZoomFilterMode::IMAGE_DISPLACEMENT_MODE:
      return std::make_unique<ImageZoomAdjustment>(resourcesDirectory, goomRand);
    case ZoomFilterMode::IMAGE_DISPLACEMENT_OF_WAVE_SQ_DIST_ANGLE_MODE0:
      return std::make_unique<FunctionOfFunction>(
          goomRand,
          "Image_of_Wave",
          std::make_unique<ImageZoomAdjustment>(resourcesDirectory, goomRand),
          std::make_unique<Wave>(Wave::Modes::SQ_DIST_ANGLE_EFFECT_MODE0, goomRand));
    case ZoomFilterMode::MOBIUS_MODE:
      return std::make_unique<Mobius>(goomRand);
    case ZoomFilterMode::NEWTON_MODE:
      return std::make_unique<Newton>(goomRand);
    case ZoomFilterMode::NORMAL_MODE:
      return std::make_unique<UniformZoomAdjustmentEffect>();
    case ZoomFilterMode::PERLIN_NOISE_MODE:
      return std::make_unique<PerlinNoise>(goomRand);
    case ZoomFilterMode::PERLIN_NOISE_OF_WAVE_SQ_DIST_ANGLE_MODE0:
      return std::make_unique<FunctionOfFunction>(
          goomRand,
          "Perlin_of_Wave",
          std::make_unique<PerlinNoise>(goomRand),
          std::make_unique<Wave>(Wave::Modes::SQ_DIST_ANGLE_EFFECT_MODE0, goomRand));
    case ZoomFilterMode::SCRUNCH_MODE:
      return std::make_unique<Scrunch>(goomRand);
    case ZoomFilterMode::SPEEDWAY_MODE0:
      return std::make_unique<Speedway>(Speedway::Modes::MODE0, goomRand);
    case ZoomFilterMode::SPEEDWAY_MODE1:
      return std::make_unique<Speedway>(Speedway::Modes::MODE1, goomRand);
    case ZoomFilterMode::SPEEDWAY_MODE2:
      return std::make_unique<Speedway>(Speedway::Modes::MODE2, goomRand);
    case ZoomFilterMode::WATER_MODE:
      return std::make_unique<UniformZoomAdjustmentEffect>();
    case ZoomFilterMode::WAVE_SQ_DIST_ANGLE_EFFECT_MODE0:
      return std::make_unique<Wave>(Wave::Modes::SQ_DIST_ANGLE_EFFECT_MODE0, goomRand);
    case ZoomFilterMode::WAVE_SQ_DIST_ANGLE_EFFECT_MODE1:
      return std::make_unique<Wave>(Wave::Modes::SQ_DIST_ANGLE_EFFECT_MODE1, goomRand);
    case ZoomFilterMode::WAVE_ATAN_ANGLE_EFFECT_MODE0:
      return std::make_unique<Wave>(Wave::Modes::ATAN_ANGLE_EFFECT_MODE0, goomRand);
    case ZoomFilterMode::WAVE_ATAN_ANGLE_EFFECT_MODE1:
      return std::make_unique<Wave>(Wave::Modes::ATAN_ANGLE_EFFECT_MODE1, goomRand);
    case ZoomFilterMode::Y_ONLY_MODE:
      return std::make_unique<YOnly>(goomRand);
  }
}

auto CreateFuncOfFuncZoomAdjustmentEffect(const ZoomFilterMode filterMode,
                                          const IGoomRand& goomRand,
                                          const std::string& resourcesDirectory)
    -> std::unique_ptr<IZoomAdjustmentEffect>
{
  auto zoomAdjustmentEffect =
      CreateFuncZoomAdjustmentEffect(filterMode, goomRand, resourcesDirectory);
  const auto funcOfMode =
      static_cast<ZoomFilterMode>(goomRand.GetRandInRange(0U, NUM<ZoomFilterMode>));
  auto funcOf = CreateFuncZoomAdjustmentEffect(funcOfMode, goomRand, resourcesDirectory);
  const auto fofName =
      std::format("{}({})", GetFilterModeName(funcOfMode), GetFilterModeName(filterMode));
  return std::make_unique<FunctionOfFunction>(
      goomRand, fofName, std::move(funcOf), std::move(zoomAdjustmentEffect));
}

} // namespace

auto CreateZoomAdjustmentEffect(const ZoomFilterMode filterMode,
                                const IGoomRand& goomRand,
                                const std::string& resourcesDirectory)
    -> std::unique_ptr<IZoomAdjustmentEffect>
{
  if (Contains(NON_FUNC_OF_FUNC_EFFECTS, filterMode))
  {
    return CreateFuncZoomAdjustmentEffect(filterMode, goomRand, resourcesDirectory);
  }

  if (static constexpr auto PROB_FUNC_OF_FUNC = 0.5F; goomRand.ProbabilityOf(PROB_FUNC_OF_FUNC))
  {
    return CreateFuncOfFuncZoomAdjustmentEffect(filterMode, goomRand, resourcesDirectory);
  }

  return CreateFuncZoomAdjustmentEffect(filterMode, goomRand, resourcesDirectory);
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
