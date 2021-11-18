#include "distance_field.h"

#include "utils/mathutils.h"
#include "utils/name_value_pairs.h"

#undef NDEBUG
#include <cassert>

#if __cplusplus <= 201402L
namespace GOOM
{
namespace VISUAL_FX
{
namespace FILTERS
{
#else
namespace GOOM::VISUAL_FX::FILTERS
{
#endif

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

DistanceField::DistanceField(IGoomRand& goomRand) noexcept
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
  m_params.xAmplitude = m_goomRand.GetRandInRange(X_AMPLITUDE_RANGE_MODE0);
  m_params.yAmplitude = m_goomRand.ProbabilityOf(PROB_XY_AMPLITUDES_EQUAL)
                            ? m_params.xAmplitude
                            : m_goomRand.GetRandInRange(Y_AMPLITUDE_RANGE_MODE0);

  m_params.xSqDistMult = m_goomRand.GetRandInRange(X_SQ_DIST_MULT_RANGE_MODE0);
  m_params.ySqDistMult = m_goomRand.ProbabilityOf(PROB_XY_SQ_DIST_MULT_EQUAL)
                             ? m_params.xSqDistMult
                             : m_goomRand.GetRandInRange(Y_SQ_DIST_MULT_RANGE_MODE0);

  m_params.xSqDistOffset = m_goomRand.GetRandInRange(X_SQ_DIST_OFFSET_RANGE_MODE0);
  m_params.ySqDistOffset = m_goomRand.ProbabilityOf(PROB_XY_SQ_DIST_OFFSET_EQUAL)
                               ? m_params.xSqDistOffset
                               : m_goomRand.GetRandInRange(Y_SQ_DIST_OFFSET_RANGE_MODE0);

  m_params.mode0 = m_goomRand.ProbabilityOf(PROB_MODE0);

  constexpr size_t NUM_DISTANCE_POINTS = 4;
  m_params.distancePoints.clear();

  if (m_goomRand.ProbabilityOf(PROB_RANDOM_DISTANCE_POINTS))
  {
    m_params.distancePoints.emplace_back(-1.0F, -1.0F);
    m_params.distancePoints.emplace_back(+1.0F, -1.0F);
    m_params.distancePoints.emplace_back(-1.0F, +1.0F);
    m_params.distancePoints.emplace_back(+1.0F, +1.0F);
  }
  else
  {
    for (size_t i = 0; i < NUM_DISTANCE_POINTS; ++i)
    {
      m_params.distancePoints.emplace_back(m_goomRand.GetRandInRange(-1.9F, 1.9F),
                                           m_goomRand.GetRandInRange(-1.9F, 1.9F));
    }
  }
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

#if __cplusplus <= 201402L
} // namespace FILTERS
} // namespace VISUAL_FX
} // namespace GOOM
#else
} // namespace GOOM::VISUAL_FX::FILTERS
#endif
