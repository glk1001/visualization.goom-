#include "distance_field.h"

#include "filter_fx/common_types.h"
#include "goom_config.h"
#include "goom_types.h"
#include "utils/math/misc.h"
#include "utils/name_value_pairs.h"

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

using UTILS::NameValuePairs;
using UTILS::MATH::IGoomRand;
using UTILS::MATH::Sq;

static constexpr auto DEFAULT_AMPLITUDE     = 0.1F;
static constexpr auto AMPLITUDE_RANGE_MODE0 = AmplitudeRange{
    {0.01F, 0.501F},
    {0.01F, 0.501F},
};
static constexpr auto AMPLITUDE_RANGE_MODE1 = AmplitudeRange{
    {0.20F, 1.01F},
    {0.20F, 1.01F},
};
static constexpr auto AMPLITUDE_RANGE_MODE2 = AmplitudeRange{
    {0.50F, 1.51F},
    {0.50F, 1.51F},
};

static constexpr auto DEFAULT_SQ_DIST_MULT     = 0.5F;
static constexpr auto SQ_DIST_MULT_RANGE_MODE0 = SqDistMultRange{
    {0.0F, 0.01F},
    {0.0F, 0.01F},
};
static constexpr auto SQ_DIST_MULT_RANGE_MODE1 = SqDistMultRange{
    {0.5F, 2.01F},
    {0.5F, 2.01F},
};
static constexpr auto SQ_DIST_MULT_RANGE_MODE2 = SqDistMultRange{
    {0.5F, 3.01F},
    {0.5F, 3.01F},
};

static constexpr auto DEFAULT_SQ_DIST_OFFSET     = 0.1F;
static constexpr auto SQ_DIST_OFFSET_RANGE_MODE0 = SqDistOffsetRange{
    {0.0F, 0.01F},
    {0.0F, 0.01F},
};
static constexpr auto SQ_DIST_OFFSET_RANGE_MODE1 = SqDistOffsetRange{
    {0.1F, 1.01F},
    {0.1F, 1.01F},
};
static constexpr auto SQ_DIST_OFFSET_RANGE_MODE2 = SqDistOffsetRange{
    {0.3F, 2.01F},
    {0.3F, 2.01F},
};

static constexpr auto PROB_XY_AMPLITUDES_EQUAL     = 0.50F;
static constexpr auto PROB_XY_SQ_DIST_MULT_EQUAL   = 0.50F;
static constexpr auto PROB_XY_SQ_DIST_OFFSET_EQUAL = 0.50F;

static constexpr auto PROB_RANDOM_DISTANCE_POINTS = 0.3F;

DistanceField::DistanceField(const Modes mode, const IGoomRand& goomRand) noexcept
  : m_mode{mode},
    m_goomRand{goomRand},
    m_params{
        {DEFAULT_AMPLITUDE,      DEFAULT_AMPLITUDE},
        {DEFAULT_SQ_DIST_MULT,   DEFAULT_SQ_DIST_MULT},
        {DEFAULT_SQ_DIST_OFFSET, DEFAULT_SQ_DIST_OFFSET},
        {}
    }
{
}

auto DistanceField::SetRandomParams() noexcept -> void
{
  if (m_mode == Modes::MODE0)
  {
    SetMode0RandomParams();
  }
  else if (m_mode == Modes::MODE1)
  {
    SetMode1RandomParams();
  }
  else
  {
    SetMode2RandomParams();
  }
}

auto DistanceField::SetMode0RandomParams() noexcept -> void
{
  SetRandomParams(AMPLITUDE_RANGE_MODE0,
                  SQ_DIST_MULT_RANGE_MODE0,
                  SQ_DIST_OFFSET_RANGE_MODE0,
                  GetDistancePoints());
}

auto DistanceField::SetMode1RandomParams() noexcept -> void
{
  SetRandomParams(AMPLITUDE_RANGE_MODE1,
                  SQ_DIST_MULT_RANGE_MODE1,
                  SQ_DIST_OFFSET_RANGE_MODE1,
                  GetDistancePoints());
}

auto DistanceField::SetMode2RandomParams() noexcept -> void
{
  SetRandomParams(AMPLITUDE_RANGE_MODE2,
                  SQ_DIST_MULT_RANGE_MODE2,
                  SQ_DIST_OFFSET_RANGE_MODE2,
                  GetDistancePoints());
}

auto DistanceField::SetRandomParams(const AmplitudeRange& amplitudeRange,
                                    const SqDistMultRange& sqDistMultRange,
                                    const SqDistOffsetRange& sqDistOffsetRange,
                                    std::vector<NormalizedCoords>&& distancePoints) noexcept -> void
{
  const auto xAmplitude = m_goomRand.GetRandInRange(amplitudeRange.xRange);
  const auto yAmplitude = m_goomRand.ProbabilityOf(PROB_XY_AMPLITUDES_EQUAL)
                              ? xAmplitude
                              : m_goomRand.GetRandInRange(amplitudeRange.yRange);

  const auto xSqDistMult = m_goomRand.GetRandInRange(sqDistMultRange.xRange);
  const auto ySqDistMult = m_goomRand.ProbabilityOf(PROB_XY_SQ_DIST_MULT_EQUAL)
                               ? xSqDistMult
                               : m_goomRand.GetRandInRange(sqDistMultRange.yRange);

  const auto xSqDistOffset = m_goomRand.GetRandInRange(sqDistOffsetRange.xRange);
  const auto ySqDistOffset = m_goomRand.ProbabilityOf(PROB_XY_SQ_DIST_OFFSET_EQUAL)
                                 ? xSqDistOffset
                                 : m_goomRand.GetRandInRange(sqDistOffsetRange.yRange);

  SetParams({
      {   xAmplitude,    yAmplitude},
      {  xSqDistMult,   ySqDistMult},
      {xSqDistOffset, ySqDistOffset},
      distancePoints
  });
}

auto DistanceField::GetDistancePoints() const noexcept -> std::vector<NormalizedCoords>
{
  auto distancePoints = std::vector<NormalizedCoords>{};

  static constexpr auto NUM_DISTANCE_POINTS = 4U;

  static constexpr auto Y_SHRINK_FACTOR = 0.67F;

  if (!m_goomRand.ProbabilityOf(PROB_RANDOM_DISTANCE_POINTS))
  {
    static constexpr auto HALF_MIN_COORD = 0.5F * NormalizedCoords::MIN_NORMALIZED_COORD;
    static constexpr auto HALF_MAX_COORD = 0.5F * NormalizedCoords::MAX_NORMALIZED_COORD;
    distancePoints.emplace_back(HALF_MIN_COORD, Y_SHRINK_FACTOR * HALF_MIN_COORD);
    distancePoints.emplace_back(HALF_MAX_COORD, Y_SHRINK_FACTOR * HALF_MIN_COORD);
    distancePoints.emplace_back(HALF_MIN_COORD, Y_SHRINK_FACTOR * HALF_MAX_COORD);
    distancePoints.emplace_back(HALF_MAX_COORD, Y_SHRINK_FACTOR * HALF_MAX_COORD);
  }
  else
  {
    static constexpr auto MIN_DISTANCE_COORD = 0.95F * NormalizedCoords::MIN_NORMALIZED_COORD;
    static constexpr auto MAX_DISTANCE_COORD = 0.95F * NormalizedCoords::MAX_NORMALIZED_COORD;
    for (auto i = 0U; i < NUM_DISTANCE_POINTS; ++i)
    {
      distancePoints.emplace_back(
          m_goomRand.GetRandInRange(MIN_DISTANCE_COORD, MAX_DISTANCE_COORD),
          Y_SHRINK_FACTOR * m_goomRand.GetRandInRange(MIN_DISTANCE_COORD, MAX_DISTANCE_COORD));
    }
  }

  return distancePoints;
}

auto DistanceField::GetClosestDistancePoint(const NormalizedCoords& coords) const noexcept
    -> RelativeDistancePoint
{
  static constexpr auto MAX_DISTANCE_SQ =
      2.0F * Sq(NormalizedCoords::MAX_NORMALIZED_COORD - NormalizedCoords::MIN_NORMALIZED_COORD);
  auto minDistanceSq       = MAX_DISTANCE_SQ;
  const auto* closestPoint = Ptr<NormalizedCoords>{nullptr};

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

auto DistanceField::GetZoomInCoefficientsEffectNameValueParams() const noexcept -> NameValuePairs
{
  return {};
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
