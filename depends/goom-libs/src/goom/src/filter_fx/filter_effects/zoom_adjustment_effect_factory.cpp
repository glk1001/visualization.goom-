#include "zoom_adjustment_effect_factory.h"

#include "filter_fx/filter_effects/adjustment_effects/amulet.h"
#include "filter_fx/filter_effects/adjustment_effects/complex_rational.h"
#include "filter_fx/filter_effects/adjustment_effects/crystal_ball.h"
#include "filter_fx/filter_effects/adjustment_effects/distance_field.h"
#include "filter_fx/filter_effects/adjustment_effects/exp_reciprocal.h"
#include "filter_fx/filter_effects/adjustment_effects/func_of_func.h"
#include "filter_fx/filter_effects/adjustment_effects/image_zoom_adjustment.h"
#include "filter_fx/filter_effects/adjustment_effects/mobius.h"
#include "filter_fx/filter_effects/adjustment_effects/newton.h"
#include "filter_fx/filter_effects/adjustment_effects/perlin_noise.h"
#include "filter_fx/filter_effects/adjustment_effects/scrunch.h"
#include "filter_fx/filter_effects/adjustment_effects/speedway.h"
#include "filter_fx/filter_effects/adjustment_effects/uniform_zoom_adjustment_effect.h"
#include "filter_fx/filter_effects/adjustment_effects/wave.h"
#include "filter_fx/filter_effects/adjustment_effects/y_only.h"
#include "filter_fx/filter_settings_service.h"
#include "filter_fx/zoom_adjustment_effect.h"
#include "goom/goom_config.h"
#include "utils/enum_utils.h"
#include "utils/math/goom_rand_base.h"

#include <format>
#include <memory>
#include <string>
#include <utility>

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

using UTILS::NUM;
using UTILS::MATH::IGoomRand;

namespace
{

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
      return nullptr;
    case ZoomFilterMode::MOBIUS_MODE:
      return std::make_unique<Mobius>(goomRand);
    case ZoomFilterMode::NEWTON_MODE:
      return std::make_unique<Newton>(goomRand);
    case ZoomFilterMode::NORMAL_MODE:
      return std::make_unique<UniformZoomAdjustmentEffect>();
    case ZoomFilterMode::PERLIN_NOISE_MODE:
      return std::make_unique<PerlinNoise>(goomRand);
    case ZoomFilterMode::PERLIN_NOISE_OF_WAVE_SQ_DIST_ANGLE_MODE0:
      return nullptr;
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

} // namespace

auto CreateZoomAdjustmentEffect(const ZoomFilterMode filterMode,
                                const IGoomRand& goomRand,
                                const std::string& resourcesDirectory)
    -> std::unique_ptr<IZoomAdjustmentEffect>
{
  static constexpr auto PROB_FUNC_OF_FUNC = 0.5F;
  if (goomRand.ProbabilityOf(PROB_FUNC_OF_FUNC))
  {
    //const auto filtMode = ZoomFilterMode::COMPLEX_RATIONAL_MODE;
    auto zoomAdjustmentEffect =
        CreateFuncZoomAdjustmentEffect(filterMode, goomRand, resourcesDirectory);
    if (zoomAdjustmentEffect != nullptr)
    {
      const auto funcOfMode =
          static_cast<ZoomFilterMode>(goomRand.GetRandInRange(0U, NUM<ZoomFilterMode>));
      //const auto funcOfMode = ZoomFilterMode::AMULET_MODE;
      auto funcOf = CreateFuncZoomAdjustmentEffect(funcOfMode, goomRand, resourcesDirectory);
      if (funcOf != nullptr)
      {
        const auto fofName =
            std::format("{}({})", GetFilterModeName(funcOfMode), GetFilterModeName(filterMode));
        return std::make_unique<FunctionOfFunction>(
            fofName, std::move(funcOf), std::move(zoomAdjustmentEffect));
      }
    }
  }

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
      //      return std::make_unique<FunctionOfFunction>(
      //          std::make_unique<Wave>(Wave::Modes::SQ_DIST_ANGLE_EFFECT_MODE0, goomRand),
      //          std::make_unique<ImageZoomAdjustment>(resourcesDirectory, goomRand));
      return std::make_unique<FunctionOfFunction>(
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

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
