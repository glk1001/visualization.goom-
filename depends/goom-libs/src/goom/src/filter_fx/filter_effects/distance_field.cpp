#include "distance_field.h"

#include "goom_config.h"
#include "utils/math/misc.h"
#include "utils/name_value_pairs.h"

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

using UTILS::NameValuePairs;
using UTILS::MATH::IGoomRand;
using UTILS::MATH::Sq;

static constexpr auto DEFAULT_AMPLITUDE       = 0.1F;
static constexpr auto X_AMPLITUDE_RANGE_MODE0 = IGoomRand::NumberRange<float>{0.01F, 1.11F};
static constexpr auto Y_AMPLITUDE_RANGE_MODE0 = IGoomRand::NumberRange<float>{0.01F, 1.11F};

static constexpr auto DEFAULT_SQ_DIST_MULT       = 0.025F;
static constexpr auto X_SQ_DIST_MULT_RANGE_MODE0 = IGoomRand::NumberRange<float>{0.01F, 0.051F};
static constexpr auto Y_SQ_DIST_MULT_RANGE_MODE0 = IGoomRand::NumberRange<float>{0.01F, 0.051F};

static constexpr auto DEFAULT_SQ_DIST_OFFSET       = 0.05F;
static constexpr auto X_SQ_DIST_OFFSET_RANGE_MODE0 = IGoomRand::NumberRange<float>{0.01F, 1.11F};
static constexpr auto Y_SQ_DIST_OFFSET_RANGE_MODE0 = IGoomRand::NumberRange<float>{0.01F, 1.11F};

static constexpr auto PROB_XY_AMPLITUDES_EQUAL     = 0.50F;
static constexpr auto PROB_XY_SQ_DIST_MULT_EQUAL   = 0.50F;
static constexpr auto PROB_XY_SQ_DIST_OFFSET_EQUAL = 0.50F;

static constexpr auto PROB_MODE0                  = 0.7F;
static constexpr auto PROB_RANDOM_DISTANCE_POINTS = 0.3F;

DistanceField::DistanceField(const IGoomRand& goomRand) noexcept
  : m_goomRand{goomRand},
    m_params{false,
             DEFAULT_AMPLITUDE,
             DEFAULT_AMPLITUDE,
             DEFAULT_SQ_DIST_MULT,
             DEFAULT_SQ_DIST_MULT,
             DEFAULT_SQ_DIST_OFFSET,
             DEFAULT_SQ_DIST_OFFSET,
             {}}
{
}

auto DistanceField::SetRandomParams() -> void
{
  const auto mode0 = m_goomRand.ProbabilityOf(PROB_MODE0);

  const auto xAmplitude = m_goomRand.GetRandInRange(X_AMPLITUDE_RANGE_MODE0);
  const auto yAmplitude = m_goomRand.ProbabilityOf(PROB_XY_AMPLITUDES_EQUAL)
                              ? xAmplitude
                              : m_goomRand.GetRandInRange(Y_AMPLITUDE_RANGE_MODE0);

  const auto xSqDistMult = m_goomRand.GetRandInRange(X_SQ_DIST_MULT_RANGE_MODE0);
  const auto ySqDistMult = m_goomRand.ProbabilityOf(PROB_XY_SQ_DIST_MULT_EQUAL)
                               ? xSqDistMult
                               : m_goomRand.GetRandInRange(Y_SQ_DIST_MULT_RANGE_MODE0);

  const auto xSqDistOffset = m_goomRand.GetRandInRange(X_SQ_DIST_OFFSET_RANGE_MODE0);
  const auto ySqDistOffset = m_goomRand.ProbabilityOf(PROB_XY_SQ_DIST_OFFSET_EQUAL)
                                 ? xSqDistOffset
                                 : m_goomRand.GetRandInRange(Y_SQ_DIST_OFFSET_RANGE_MODE0);

  auto distancePoints = std::vector<NormalizedCoords>{};

  static constexpr auto NUM_DISTANCE_POINTS = 4U;

  if (!m_goomRand.ProbabilityOf(PROB_RANDOM_DISTANCE_POINTS))
  {
    static constexpr auto HALF_MIN_COORD = 0.5F * NormalizedCoords::MIN_NORMALIZED_COORD;
    static constexpr auto HALF_MAX_COORD = 0.5F * NormalizedCoords::MAX_NORMALIZED_COORD;
    distancePoints.emplace_back(HALF_MIN_COORD, HALF_MIN_COORD);
    distancePoints.emplace_back(HALF_MAX_COORD, HALF_MIN_COORD);
    distancePoints.emplace_back(HALF_MIN_COORD, HALF_MAX_COORD);
    distancePoints.emplace_back(HALF_MAX_COORD, HALF_MAX_COORD);
  }
  else
  {
    static constexpr auto MIN_DISTANCE_COORD = 0.95F * NormalizedCoords::MIN_NORMALIZED_COORD;
    static constexpr auto MAX_DISTANCE_COORD = 0.95F * NormalizedCoords::MAX_NORMALIZED_COORD;
    for (auto i = 0U; i < NUM_DISTANCE_POINTS; ++i)
    {
      distancePoints.emplace_back(
          m_goomRand.GetRandInRange(MIN_DISTANCE_COORD, MAX_DISTANCE_COORD),
          m_goomRand.GetRandInRange(MIN_DISTANCE_COORD, MAX_DISTANCE_COORD));
    }
  }

  SetParams({mode0,
             xAmplitude,
             yAmplitude,
             xSqDistMult,
             ySqDistMult,
             xSqDistOffset,
             ySqDistOffset,
             distancePoints});
}

auto DistanceField::GetClosestDistancePoint(const NormalizedCoords& coords) const
    -> RelativeDistancePoint
{
  static constexpr auto MAX_DISTANCE_SQ =
      2.0F * Sq(NormalizedCoords::MAX_NORMALIZED_COORD - NormalizedCoords::MIN_NORMALIZED_COORD);
  auto minDistanceSq                   = MAX_DISTANCE_SQ;
  const NormalizedCoords* closestPoint = nullptr;

  for (const auto& distancePoint : m_params.distancePoints)
  {
    const auto distanceSq = GetSqDistance(coords, distancePoint);
    if (distanceSq < minDistanceSq)
    {
      minDistanceSq = distanceSq;
      closestPoint  = &distancePoint;
    }
  }
  Ensures(closestPoint != nullptr);

  return {minDistanceSq, *closestPoint};
}

auto DistanceField::GetSpeedCoefficientsEffectNameValueParams() const -> NameValuePairs
{
  return {};
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
