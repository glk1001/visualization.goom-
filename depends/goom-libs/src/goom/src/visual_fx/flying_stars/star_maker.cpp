#include "star_maker.h"

#include "goom/goom_config.h"
#include "goom/point2d.h"
#include "star_colors_maker.h"
#include "star_types_base.h"
#include "stars.h"
#include "utils/math/goom_rand_base.h"
#include "utils/t_values.h"

#include <cmath>
#include <cstdint>

namespace GOOM::VISUAL_FX::FLYING_STARS
{

StarMaker::StarMaker(const UTILS::MATH::IGoomRand& goomRand) noexcept : m_goomRand{&goomRand}
{
}

auto StarMaker::StartNewCluster(const IStarType& starType,
                                const uint32_t numStarsInCluster,
                                const StarProperties& starProperties) noexcept -> void
{
  Expects(numStarsInCluster > 0);

  m_starType        = &starType;
  m_starSetupParams = GetStarSetupParams(starProperties);
  m_numStarsToMake  = numStarsInCluster;
  m_withinClusterT.SetNumSteps(numStarsInCluster);
  m_withinClusterT.Reset();
}

auto StarMaker::GetStarSetupParams(const StarProperties& starProperties) const noexcept
    -> IStarType::SetupParams
{
  auto setupParams = m_starType->GetRandomizedSetupParams(starProperties.defaultPathLength);

  setupParams.nominalPathLength *= starProperties.heightRatio;
  setupParams.nominalPathLength *= starProperties.nominalPathLengthFactor;

  return setupParams;
}

auto StarMaker::MakeNewStar() noexcept -> Star
{
  Expects(m_numStarsToMake > 0);

  const auto newStarPathAngle = m_starType->GetRandomizedStarPathAngle(m_starSetupParams.startPos);
  const auto newStarParams    = GetNewStarParams(newStarPathAngle);

  auto newStar =
      Star{newStarParams, m_starType->GetStarColorsMaker().GetNewStarColors(m_withinClusterT())};

  --m_numStarsToMake;
  m_withinClusterT.Increment();

  return newStar;
}

auto StarMaker::GetNewStarParams(const float starPathAngle) const noexcept -> Star::Params
{
  const auto initialPosition = ToPoint2dFlt(m_starSetupParams.startPos);

  static constexpr auto MIN_PATH_LENGTH = 0.01F;
  static constexpr auto MAX_PATH_LENGTH = 2.00F;
  const auto starPathLength             = m_starSetupParams.nominalPathLength *
                              m_goomRand->GetRandInRange(MIN_PATH_LENGTH, MAX_PATH_LENGTH);
  static constexpr auto LENGTH_OFFSET = -0.2F;
  const auto initialVelocity          = Vec2dFlt{starPathLength * std::cos(starPathAngle),
                                        LENGTH_OFFSET + (starPathLength * std::sin(starPathAngle))};

  const auto initialAcceleration = Vec2dFlt{m_starSetupParams.sideWind, m_starSetupParams.gravity};

  return {
      initialPosition, initialVelocity, initialAcceleration, 0.0F, m_starSetupParams.starTAgeInc};
}

} //namespace GOOM::VISUAL_FX::FLYING_STARS
