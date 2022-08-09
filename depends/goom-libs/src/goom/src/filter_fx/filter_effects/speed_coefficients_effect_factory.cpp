#include "speed_coefficients_effect_factory.h"

#include "amulet.h"
#include "crystal_ball.h"
#include "distance_field.h"
#include "filter_fx/filter_settings_service.h"
#include "image_speed_coeffs.h"
#include "scrunch.h"
#include "simple_speed_coefficients_effect.h"
#include "speedway.h"
#include "utils/math/goom_rand_base.h"
#include "wave.h"
#include "y_only.h"

#include <memory>

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

using GOOM::UTILS::MATH::IGoomRand;

auto CreateSpeedCoefficientsEffect(const ZoomFilterMode filterMode,
                                   const IGoomRand& goomRand,
                                   const std::string& resourcesDirectory)
    -> std::shared_ptr<ISpeedCoefficientsEffect>
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
    case ZoomFilterMode::HYPERCOS_MODE0:
    case ZoomFilterMode::HYPERCOS_MODE1:
    case ZoomFilterMode::HYPERCOS_MODE2:
    case ZoomFilterMode::HYPERCOS_MODE3:
      return std::make_shared<SimpleSpeedCoefficientsEffect>();
    case ZoomFilterMode::IMAGE_DISPLACEMENT_MODE:
      return std::make_shared<ImageSpeedCoefficients>(resourcesDirectory, goomRand);
    case ZoomFilterMode::NORMAL_MODE:
      return std::make_shared<SimpleSpeedCoefficientsEffect>();
    case ZoomFilterMode::SCRUNCH_MODE:
      return std::make_shared<Scrunch>(goomRand);
    case ZoomFilterMode::SPEEDWAY_MODE0:
      return std::make_shared<Speedway>(Speedway::Modes::MODE0, goomRand);
    case ZoomFilterMode::SPEEDWAY_MODE1:
      return std::make_shared<Speedway>(Speedway::Modes::MODE1, goomRand);
    case ZoomFilterMode::SPEEDWAY_MODE2:
      return std::make_shared<Speedway>(Speedway::Modes::MODE2, goomRand);
    case ZoomFilterMode::WATER_MODE:
      return std::make_shared<SimpleSpeedCoefficientsEffect>();
    case ZoomFilterMode::WAVE_MODE0:
      return std::make_shared<Wave>(Wave::Modes::MODE0, goomRand);
    case ZoomFilterMode::WAVE_MODE1:
      return std::make_shared<Wave>(Wave::Modes::MODE1, goomRand);
    case ZoomFilterMode::Y_ONLY_MODE:
      return std::make_shared<YOnly>(goomRand);
    default:
      throw std::logic_error("Invalid filter mode.");
  }
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
