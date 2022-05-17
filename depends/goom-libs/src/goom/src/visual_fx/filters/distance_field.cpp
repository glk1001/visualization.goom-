#include "distance_field.h"

#include "goom_config.h"
#include "utils/math/misc.h"
#include "utils/name_value_pairs.h"

namespace GOOM::VISUAL_FX::FILTERS
{

using UTILS::NameValuePairs;
using UTILS::MATH::IGoomRand;
using UTILS::MATH::Sq;

static constexpr float DEFAULT_AMPLITUDE = 0.1F;
static constexpr IGoomRand::NumberRange<float> X_AMPLITUDE_RANGE_MODE0 = {0.01F, 0.501F};
static constexpr IGoomRand::NumberRange<float> Y_AMPLITUDE_RANGE_MODE0 = {0.01F, 0.501F};

static constexpr float DEFAULT_SQ_DIST_MULT = 0.025F;
static constexpr IGoomRand::NumberRange<float> X_SQ_DIST_MULT_RANGE_MODE0 = {0.01F, 0.051F};
static constexpr IGoomRand::NumberRange<float> Y_SQ_DIST_MULT_RANGE_MODE0 = {0.01F, 0.051F};

static constexpr float DEFAULT_SQ_DIST_OFFSET = 0.05F;
static constexpr IGoomRand::NumberRange<float> X_SQ_DIST_OFFSET_RANGE_MODE0 = {0.01F, 0.11F};
static constexpr IGoomRand::NumberRange<float> Y_SQ_DIST_OFFSET_RANGE_MODE0 = {0.01F, 0.11F};

static constexpr float PROB_XY_AMPLITUDES_EQUAL = 1.00F;
static constexpr float PROB_XY_SQ_DIST_MULT_EQUAL = 1.00F;
static constexpr float PROB_XY_SQ_DIST_OFFSET_EQUAL = 1.00F;

static constexpr float PROB_MODE0 = 0.05F;
static constexpr float PROB_RANDOM_DISTANCE_POINTS = 0.3F;

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

void DistanceField::SetRandomParams()
{
  const bool mode0 = m_goomRand.ProbabilityOf(PROB_MODE0);

  const float xAmplitude = m_goomRand.GetRandInRange(X_AMPLITUDE_RANGE_MODE0);
  const float yAmplitude = m_goomRand.ProbabilityOf(PROB_XY_AMPLITUDES_EQUAL)
                               ? xAmplitude
                               : m_goomRand.GetRandInRange(Y_AMPLITUDE_RANGE_MODE0);

  const float xSqDistMult = m_goomRand.GetRandInRange(X_SQ_DIST_MULT_RANGE_MODE0);
  const float ySqDistMult = m_goomRand.ProbabilityOf(PROB_XY_SQ_DIST_MULT_EQUAL)
                                ? xSqDistMult
                                : m_goomRand.GetRandInRange(Y_SQ_DIST_MULT_RANGE_MODE0);

  const float xSqDistOffset = m_goomRand.GetRandInRange(X_SQ_DIST_OFFSET_RANGE_MODE0);
  const float ySqDistOffset = m_goomRand.ProbabilityOf(PROB_XY_SQ_DIST_OFFSET_EQUAL)
                                  ? xSqDistOffset
                                  : m_goomRand.GetRandInRange(Y_SQ_DIST_OFFSET_RANGE_MODE0);

  std::vector<NormalizedCoords> distancePoints{};

  static constexpr size_t NUM_DISTANCE_POINTS = 4;

  if (!m_goomRand.ProbabilityOf(PROB_RANDOM_DISTANCE_POINTS))
  {
    static constexpr float HALF_MIN_COORD = 0.5F * NormalizedCoords::MIN_NORMALIZED_COORD;
    static constexpr float HALF_MAX_COORD = 0.5F * NormalizedCoords::MAX_NORMALIZED_COORD;
    distancePoints.emplace_back(HALF_MIN_COORD, HALF_MIN_COORD);
    distancePoints.emplace_back(HALF_MAX_COORD, HALF_MIN_COORD);
    distancePoints.emplace_back(HALF_MIN_COORD, HALF_MAX_COORD);
    distancePoints.emplace_back(HALF_MAX_COORD, HALF_MAX_COORD);
  }
  else
  {
    static constexpr float MIN_DISTANCE_COORD = 0.95F * NormalizedCoords::MIN_NORMALIZED_COORD;
    static constexpr float MAX_DISTANCE_COORD = 0.95F * NormalizedCoords::MAX_NORMALIZED_COORD;
    for (size_t i = 0; i < NUM_DISTANCE_POINTS; ++i)
    {
      distancePoints.emplace_back(
          m_goomRand.GetRandInRange(MIN_DISTANCE_COORD, MAX_DISTANCE_COORD),
          m_goomRand.GetRandInRange(MIN_DISTANCE_COORD, MAX_DISTANCE_COORD));
    }
  }

  SetParams({mode0, xAmplitude, yAmplitude, xSqDistMult, ySqDistMult, xSqDistOffset, ySqDistOffset,
             distancePoints});
}

auto DistanceField::GetClosestDistancePoint(const NormalizedCoords& coords) const
    -> RelativeDistancePoint
{
  static constexpr float MAX_DISTANCE_SQ =
      2.0F * Sq(NormalizedCoords::MAX_NORMALIZED_COORD - NormalizedCoords::MIN_NORMALIZED_COORD);
  float minDistanceSq = MAX_DISTANCE_SQ;
  const NormalizedCoords* closestPoint = nullptr;

  for (const auto& distancePoint : m_params.distancePoints)
  {
    const float distanceSq = GetSqDistance(coords, distancePoint);
    if (distanceSq < minDistanceSq)
    {
      minDistanceSq = distanceSq;
      closestPoint = &distancePoint;
    }
  }
  Ensures(closestPoint != nullptr);

  return {minDistanceSq, *closestPoint};
}

auto DistanceField::GetSpeedCoefficientsEffectNameValueParams() const -> NameValuePairs
{
  return {};
}

} // namespace GOOM::VISUAL_FX::FILTERS
