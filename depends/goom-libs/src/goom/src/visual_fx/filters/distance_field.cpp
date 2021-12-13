#include "distance_field.h"

#include "utils/mathutils.h"
#include "utils/name_value_pairs.h"

#undef NDEBUG
#include <cassert>

namespace GOOM::VISUAL_FX::FILTERS
{

using UTILS::IGoomRand;
using UTILS::NameValuePairs;
using UTILS::Sq;
using UTILS::SqDistance;

constexpr float DEFAULT_AMPLITUDE = 0.1F;
constexpr IGoomRand::NumberRange<float> X_AMPLITUDE_RANGE_MODE0 = {0.01F, 0.501F};
constexpr IGoomRand::NumberRange<float> Y_AMPLITUDE_RANGE_MODE0 = {0.01F, 0.501F};

constexpr float DEFAULT_SQ_DIST_MULT = 0.025F;
constexpr IGoomRand::NumberRange<float> X_SQ_DIST_MULT_RANGE_MODE0 = {0.01F, 0.051F};
constexpr IGoomRand::NumberRange<float> Y_SQ_DIST_MULT_RANGE_MODE0 = {0.01F, 0.051F};

constexpr float DEFAULT_SQ_DIST_OFFSET = 0.05F;
constexpr IGoomRand::NumberRange<float> X_SQ_DIST_OFFSET_RANGE_MODE0 = {0.01F, 0.11F};
constexpr IGoomRand::NumberRange<float> Y_SQ_DIST_OFFSET_RANGE_MODE0 = {0.01F, 0.11F};

constexpr float PROB_XY_AMPLITUDES_EQUAL = 1.00F;
constexpr float PROB_XY_SQ_DIST_MULT_EQUAL = 1.00F;
constexpr float PROB_XY_SQ_DIST_OFFSET_EQUAL = 1.00F;

constexpr float PROB_MODE0 = 0.05F;
constexpr float PROB_RANDOM_DISTANCE_POINTS = 0.7F;

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

  constexpr size_t NUM_DISTANCE_POINTS = 4;

  if (m_goomRand.ProbabilityOf(PROB_RANDOM_DISTANCE_POINTS))
  {
    distancePoints.emplace_back(-1.0F, -1.0F);
    distancePoints.emplace_back(+1.0F, -1.0F);
    distancePoints.emplace_back(-1.0F, +1.0F);
    distancePoints.emplace_back(+1.0F, +1.0F);
  }
  else
  {
    for (size_t i = 0; i < NUM_DISTANCE_POINTS; ++i)
    {
      distancePoints.emplace_back(m_goomRand.GetRandInRange(-1.9F, 1.9F),
                                  m_goomRand.GetRandInRange(-1.9F, 1.9F));
    }
  }

  SetParams({mode0, xAmplitude, yAmplitude, xSqDistMult, ySqDistMult, xSqDistOffset, ySqDistOffset,
             distancePoints});
}

auto DistanceField::GetClosestDistancePoint(const NormalizedCoords& coords) const
    -> RelativeDistancePoint
{
  const NormalizedCoords* closestPoint{};
  float minDistanceSq = 100.0F * Sq(NormalizedCoords::MAX_NORMALIZED_COORD);
  for (const auto& distancePoint : m_params.distancePoints)
  {
    const float distanceSq = GetSqDistance(coords, distancePoint);
    if (distanceSq < minDistanceSq)
    {
      minDistanceSq = distanceSq;
      closestPoint = &distancePoint;
    }
  }
  assert(closestPoint != nullptr);
  return {minDistanceSq, *closestPoint};
}

auto DistanceField::GetSpeedCoefficientsEffectNameValueParams() const -> NameValuePairs
{
  return {};
}

} // namespace GOOM::VISUAL_FX::FILTERS
