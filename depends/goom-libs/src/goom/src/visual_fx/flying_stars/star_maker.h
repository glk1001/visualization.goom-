#pragma once

#include "star_types_base.h"
#include "stars.h"
#include "utils/math/goom_rand_base.h"
#include "utils/t_values.h"

#include <cstdint>

namespace GOOM::VISUAL_FX::FLYING_STARS
{

class StarMaker
{
public:
  explicit StarMaker(const UTILS::MATH::IGoomRand& goomRand) noexcept;

  struct StarProperties
  {
    float heightRatio{};
    float defaultPathLength{};
    float nominalPathLengthFactor{};
  };
  auto StartNewCluster(const IStarType& starType,
                       uint32_t numStarsInCluster,
                       const StarProperties& starProperties) noexcept -> void;

  [[nodiscard]] auto MoreStarsToMake() const noexcept -> bool;
  [[nodiscard]] auto MakeNewStar() noexcept -> Star;

private:
  const UTILS::MATH::IGoomRand* m_goomRand;
  const IStarType* m_starType{};
  uint32_t m_numStarsToMake = 0U;
  IStarType::SetupParams m_starSetupParams{};
  UTILS::TValue m_withinClusterT{
      {UTILS::TValue::StepType::SINGLE_CYCLE, 1U}
  };

  [[nodiscard]] auto GetStarSetupParams(const StarProperties& starProperties) const noexcept
      -> IStarType::SetupParams;
  [[nodiscard]] auto GetNewStarParams(float starPathAngle) const noexcept -> Star::Params;
};

inline auto StarMaker::MoreStarsToMake() const noexcept -> bool
{
  return m_numStarsToMake > 0;
}

} //namespace GOOM::VISUAL_FX::FLYING_STARS
