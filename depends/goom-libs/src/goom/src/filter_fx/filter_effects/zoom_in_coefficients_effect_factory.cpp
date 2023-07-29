#include "zoom_in_coefficients_effect_factory.h"

#include "filter_fx/filter_effects/the_effects/amulet.h"
#include "filter_fx/filter_effects/the_effects/crystal_ball.h"
#include "filter_fx/filter_effects/the_effects/distance_field.h"
#include "filter_fx/filter_effects/the_effects/exp_reciprocal.h"
#include "filter_fx/filter_effects/the_effects/image_zoom_in_coeffs.h"
#include "filter_fx/filter_effects/the_effects/scrunch.h"
#include "filter_fx/filter_effects/the_effects/speedway.h"
#include "filter_fx/filter_effects/the_effects/uniform_zoom_in_coefficients_effect.h"
#include "filter_fx/filter_effects/the_effects/wave.h"
#include "filter_fx/filter_effects/the_effects/y_only.h"
#include "filter_fx/filter_settings_service.h"
#include "goom_config.h"
#include "utils/math/goom_rand_base.h"

#include <memory>

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

using GOOM::UTILS::MATH::IGoomRand;

auto CreateZoomInCoefficientsEffect(const ZoomFilterMode filterMode,
                                    const IGoomRand& goomRand,
                                    const std::string& resourcesDirectory)
    -> std::shared_ptr<IZoomInCoefficientsEffect>
{
  switch (filterMode)
  {
    case ZoomFilterMode::AMULET_MODE:
      return std::make_shared<Amulet>(goomRand);
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
      return std::make_shared<UniformZoomInCoefficientsEffect>();
    case ZoomFilterMode::IMAGE_DISPLACEMENT_MODE:
      return std::make_shared<ImageZoomInCoefficients>(resourcesDirectory, goomRand);
    case ZoomFilterMode::NORMAL_MODE:
      return std::make_shared<UniformZoomInCoefficientsEffect>();
    case ZoomFilterMode::SCRUNCH_MODE:
      return std::make_shared<Scrunch>(goomRand);
    case ZoomFilterMode::SPEEDWAY_MODE0:
      return std::make_shared<Speedway>(Speedway::Modes::MODE0, goomRand);
    case ZoomFilterMode::SPEEDWAY_MODE1:
      return std::make_shared<Speedway>(Speedway::Modes::MODE1, goomRand);
    case ZoomFilterMode::SPEEDWAY_MODE2:
      return std::make_shared<Speedway>(Speedway::Modes::MODE2, goomRand);
    case ZoomFilterMode::WATER_MODE:
      return std::make_shared<UniformZoomInCoefficientsEffect>();
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
    default:
      FailFast();
  }
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
