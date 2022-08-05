#include "star_types.h"

#include "goom_config.h"
#include "goom_plugin_info.h"
#include "point2d.h"
#include "stars.h"
#include "utils/math/goom_rand_base.h"
#include "utils/math/misc.h"

#include <memory>

namespace GOOM::VISUAL_FX::FLYING_STARS
{

using STD20::pi;
using UTILS::MATH::IGoomRand;
using UTILS::MATH::Sq;
using UTILS::MATH::THIRD_PI;
using UTILS::MATH::TWO_PI;
using UTILS::MATH::U_HALF;

IStarType::IStarType(const PluginInfo& goomInfo, const IGoomRand& goomRand) noexcept
  : m_goomInfo{goomInfo},
    m_goomRand{goomRand},
    m_halfWidth{static_cast<int32_t>(U_HALF * goomInfo.GetScreenWidth())},
    m_halfHeight{static_cast<int32_t>(U_HALF * goomInfo.GetScreenHeight())},
    m_xMax{static_cast<float>(goomInfo.GetScreenWidth() - 1)}
{
}

auto IStarType::UpdateWindAndGravity() noexcept -> void
{
  if (static constexpr auto PROB_NEW_WIND_AND_GRAVITY = 0.10F;
      not m_goomRand.ProbabilityOf(PROB_NEW_WIND_AND_GRAVITY))
  {
    return;
  }

  m_minSideWind = m_goomRand.GetRandInRange(MIN_MIN_SIDE_WIND, MAX_MIN_SIDE_WIND);
  m_maxSideWind = m_goomRand.GetRandInRange(MIN_MAX_SIDE_WIND, MAX_MAX_SIDE_WIND);
  m_minGravity  = m_goomRand.GetRandInRange(MIN_MIN_GRAVITY, MAX_MIN_GRAVITY);
  m_maxGravity  = m_goomRand.GetRandInRange(MIN_MAX_GRAVITY, MAX_MAX_GRAVITY);
}

auto IStarType::UpdateFixedColorMapNames() noexcept -> void
{
  m_fixedMainColorMapName = m_weightedMainColorMaps->GetRandomColorMapName();
  m_fixedLowColorMapName  = m_weightedLowColorMaps->GetRandomColorMapName();
}

auto IStarType::SetWeightedMainColorMaps(
    const std::shared_ptr<const COLOR::RandomColorMaps> weightedColorMaps) noexcept -> void
{
  m_weightedMainColorMaps = weightedColorMaps;
}

auto IStarType::SetWeightedLowColorMaps(
    const std::shared_ptr<const COLOR::RandomColorMaps> weightedColorMaps) noexcept -> void
{
  m_weightedLowColorMaps = weightedColorMaps;
}

StarTypesContainer::StarTypesContainer(const PluginInfo& goomInfo,
                                       const IGoomRand& goomRand) noexcept
  : m_goomInfo{goomInfo}, m_goomRand{goomRand}
{
}

auto StarTypesContainer::GetRandomStarType() noexcept -> IStarType*
{
  return m_starTypesList.at(static_cast<uint32_t>(m_weightedStarTypes.GetRandomWeighted())).get();
}

auto StarTypesContainer::SetWeightedMainColorMaps(
    const uint32_t starTypeId,
    std::shared_ptr<const COLOR::RandomColorMaps> weightedColorMaps) noexcept -> void
{
  Expects(starTypeId < StarTypesContainer::NUM_STAR_TYPES);
  m_starTypesList.at(starTypeId)->SetWeightedMainColorMaps(weightedColorMaps);
}

auto StarTypesContainer::SetWeightedLowColorMaps(
    const uint32_t starTypeId,
    std::shared_ptr<const COLOR::RandomColorMaps> weightedColorMaps) noexcept -> void
{
  Expects(starTypeId < StarTypesContainer::NUM_STAR_TYPES);
  m_starTypesList.at(starTypeId)->SetWeightedLowColorMaps(weightedColorMaps);
}

auto StarTypesContainer::GetCurrentColorMapsNames() const noexcept -> std::vector<std::string>
{
  auto colorMapsNames = std::vector<std::string>{};

  for (const auto& starType : m_starTypesList)
  {
    colorMapsNames.emplace_back(starType->GetWeightedMainColorMaps().GetColorMapsName());
    colorMapsNames.emplace_back(starType->GetWeightedLowColorMaps().GetColorMapsName());
  }

  return colorMapsNames;
}

auto StarTypesContainer::SetZoomMidpoint(const Point2dInt& zoomMidpoint) noexcept -> void
{
  for (auto& starType : m_starTypesList)
  {
    starType->SetZoomMidpoint(zoomMidpoint);
  }
}

auto FireworksStarType::GetRandomizedSetupParams(const float defaultPathLength) const noexcept
    -> SetupParams
{
  SetupParams setupParams;

  const auto rSq = Sq(GetHalfHeight() / 2);
  while (true)
  {
    setupParams.startPos = {
        static_cast<int32_t>(GetGoomRand().GetNRand(GetGoomInfo().GetScreenWidth())),
        static_cast<int32_t>(GetGoomRand().GetNRand(GetGoomInfo().GetScreenHeight())),
    };
    const auto sqDist = SqDistance(setupParams.startPos, GetZoomMidpoint());
    if (sqDist < rSq)
    {
      break;
    }
  }

  static constexpr auto LENGTH_FACTOR = 1.0F;
  setupParams.nominalPathLength       = LENGTH_FACTOR * defaultPathLength;
  setupParams.vage = Star::MAX_INITIAL_AGE * (1.0F - GetGoomInfo().GetSoundEvents().GetGoomPower());

  static constexpr auto INITIAL_WIND_FACTOR = 0.1F;
  setupParams.sideWind =
      INITIAL_WIND_FACTOR * GetGoomRand().GetRandInRange(GetMinSideWind(), GetMaxSideWind());

  static constexpr auto INITIAL_GRAVITY_FACTOR = 0.4F;
  setupParams.gravity =
      INITIAL_GRAVITY_FACTOR * GetGoomRand().GetRandInRange(GetMinGravity(), GetMaxGravity());

  static constexpr auto MAX_STAR_AGE_FACTOR = 2.0F / 3.0F;
  setupParams.maxStarAge =
      MAX_STAR_AGE_FACTOR * GetGoomRand().GetRandInRange(MIN_MAX_STAR_AGE, MAX_MAX_STAR_AGE);

  return setupParams;
}

auto FireworksStarType::GetRandomizedStarPathAngle(
    [[maybe_unused]] const Point2dInt& startPos) const noexcept -> float
{
  static constexpr auto MIN_FIREWORKS_ANGLE = 0.0F;
  static constexpr auto MAX_FIREWORKS_ANGLE = TWO_PI;

  return GetGoomRand().GetRandInRange(MIN_FIREWORKS_ANGLE, MAX_FIREWORKS_ANGLE);
}

auto RainStarType::GetRandomizedSetupParams(const float defaultPathLength) const noexcept
    -> SetupParams
{
  SetupParams setupParams;

  const auto x0          = static_cast<int32_t>(GetGoomInfo().GetScreenWidth() / 25);
  setupParams.startPos.x = GetGoomRand().GetRandInRange(
      x0, static_cast<int32_t>(GetGoomInfo().GetScreenWidth()) - x0);

  static constexpr auto MIN_Y = 3;
  static constexpr auto MAX_Y = 63;
  setupParams.startPos.y      = -GetGoomRand().GetRandInRange(MIN_Y, MAX_Y + 1);

  static constexpr auto LENGTH_FACTOR = 1.5F;
  setupParams.nominalPathLength       = LENGTH_FACTOR * defaultPathLength;

  static constexpr auto INITIAL_VAGE = 0.002F;
  setupParams.vage                   = INITIAL_VAGE;

  static constexpr auto INITIAL_WIND_FACTOR = 1.0F;
  setupParams.sideWind =
      INITIAL_WIND_FACTOR * GetGoomRand().GetRandInRange(GetMinSideWind(), GetMaxSideWind());

  static constexpr auto INITIAL_GRAVITY_FACTOR = 0.4F;
  setupParams.gravity =
      INITIAL_GRAVITY_FACTOR * GetGoomRand().GetRandInRange(GetMinGravity(), GetMaxGravity());

  setupParams.maxStarAge = GetGoomRand().GetRandInRange(MIN_MAX_STAR_AGE, MAX_MAX_STAR_AGE);

  return setupParams;
}

auto RainStarType::GetRandomizedStarPathAngle(const Point2dInt& startPos) const noexcept -> float
{
  static constexpr auto MIN_RAIN_ANGLE     = 0.1F;
  static constexpr auto MAX_MIN_RAIN_ANGLE = THIRD_PI;
  static constexpr auto MAX_RAIN_ANGLE     = pi - 0.1F;

  const auto xFactor  = static_cast<float>(startPos.x) / GetXMax();
  const auto minAngle = STD20::lerp(MIN_RAIN_ANGLE, MAX_MIN_RAIN_ANGLE - 0.1F, 1.0F - xFactor);
  const auto maxAngle = STD20::lerp(MAX_MIN_RAIN_ANGLE + 0.1F, MAX_RAIN_ANGLE, xFactor);

  return GetGoomRand().GetRandInRange(minAngle, maxAngle);
}

auto FountainStarType::GetRandomizedSetupParams(const float defaultPathLength) const noexcept
    -> SetupParams
{
  SetupParams setupParams;

  const auto x0          = GetHalfWidth() / 5;
  setupParams.startPos.x = GetGoomRand().GetRandInRange(GetHalfWidth() - x0, GetHalfWidth() + x0);

  static constexpr auto MIN_Y = 3U;
  static constexpr auto MAX_Y = 63U;
  setupParams.startPos.y      = static_cast<int32_t>(GetGoomInfo().GetScreenHeight() +
                                                GetGoomRand().GetRandInRange(MIN_Y, MAX_Y + 1));

  setupParams.nominalPathLength = 1.0F + defaultPathLength;

  static constexpr auto INITIAL_VAGE = 0.001F;
  setupParams.vage                   = INITIAL_VAGE;

  static constexpr auto INITIAL_WIND_FACTOR = 1.0F;
  setupParams.sideWind =
      INITIAL_WIND_FACTOR * GetGoomRand().GetRandInRange(GetMinSideWind(), GetMaxSideWind());

  static constexpr auto INITIAL_GRAVITY_FACTOR = 1.0F;
  setupParams.gravity =
      INITIAL_GRAVITY_FACTOR * GetGoomRand().GetRandInRange(GetMinGravity(), GetMaxGravity());

  setupParams.maxStarAge = GetGoomRand().GetRandInRange(MIN_MAX_STAR_AGE, MAX_MAX_STAR_AGE);

  return setupParams;
}

auto FountainStarType::GetRandomizedStarPathAngle(const Point2dInt& startPos) const noexcept
    -> float
{
  static constexpr auto MIN_FOUNTAIN_ANGLE     = pi + 0.1F;
  static constexpr auto MAX_MIN_FOUNTAIN_ANGLE = pi + THIRD_PI;
  static constexpr auto MAX_FOUNTAIN_ANGLE     = TWO_PI - 0.1F;

  const auto xFactor = static_cast<float>(startPos.x) / GetXMax();
  const auto minAngle =
      STD20::lerp(MIN_FOUNTAIN_ANGLE, MAX_MIN_FOUNTAIN_ANGLE - 0.1F, 1.0F - xFactor);
  const auto maxAngle = STD20::lerp(MAX_MIN_FOUNTAIN_ANGLE + 0.1F, MAX_FOUNTAIN_ANGLE, xFactor);

  return GetGoomRand().GetRandInRange(minAngle, maxAngle);
}

} //namespace GOOM::VISUAL_FX::FLYING_STARS
