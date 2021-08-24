#include "filter_crystal_ball.h"

#include "goomutils/goomrand.h"

#undef NDEBUG
#include <cassert>

#if __cplusplus <= 201402L
namespace GOOM
{
namespace FILTERS
{
#else
namespace GOOM::FILTERS
{
#endif

using UTILS::GetRandInRange;
using UTILS::NumberRange;
using UTILS::ProbabilityOf;

constexpr float DEFAULT_AMPLITUDE = 0.1F;
constexpr NumberRange<float> X_AMPLITUDE_RANGE_MODE0 = {0.001F, 0.501F};
constexpr NumberRange<float> Y_AMPLITUDE_RANGE_MODE0 = {0.001F, 0.501F};
constexpr NumberRange<float> X_AMPLITUDE_RANGE_MODE1 = {0.001F, 15.0F};
constexpr NumberRange<float> Y_AMPLITUDE_RANGE_MODE1 = {0.001F, 15.0F};

constexpr float DEFAULT_SQ_DIST_MULT = 0.025F;
constexpr NumberRange<float> X_SQ_DIST_MULT_RANGE_MODE0 = {0.001F, 0.051F};
constexpr NumberRange<float> Y_SQ_DIST_MULT_RANGE_MODE0 = {0.001F, 0.051F};
constexpr NumberRange<float> X_SQ_DIST_MULT_RANGE_MODE1 = {0.001F, 3.5F};
constexpr NumberRange<float> Y_SQ_DIST_MULT_RANGE_MODE1 = {0.001F, 3.5F};

constexpr float DEFAULT_SQ_DIST_OFFSET = 0.05F;
constexpr NumberRange<float> X_SQ_DIST_OFFSET_RANGE_MODE0 = {0.001F, 0.11F};
constexpr NumberRange<float> Y_SQ_DIST_OFFSET_RANGE_MODE0 = {0.001F, 0.11F};
constexpr NumberRange<float> X_SQ_DIST_OFFSET_RANGE_MODE1 = {0.001F, 1.5F};
constexpr NumberRange<float> Y_SQ_DIST_OFFSET_RANGE_MODE1 = {0.001F, 1.5F};

constexpr float PROB_XY_AMPLITUDES_EQUAL = 0.5F;
constexpr float PROB_XY_SQ_DIST_MULT_EQUAL = 0.5F;
constexpr float PROB_XY_SQ_DIST_OFFSET_EQUAL = 0.5F;

CrystalBall::CrystalBall() noexcept
  : m_params{DEFAULT_AMPLITUDE,    DEFAULT_AMPLITUDE,      DEFAULT_SQ_DIST_MULT,
             DEFAULT_SQ_DIST_MULT, DEFAULT_SQ_DIST_OFFSET, DEFAULT_SQ_DIST_OFFSET}
{
}

void CrystalBall::SetMode0RandomParams()
{
  SetRandomParams(X_AMPLITUDE_RANGE_MODE0, Y_AMPLITUDE_RANGE_MODE0, X_SQ_DIST_MULT_RANGE_MODE0,
                  Y_SQ_DIST_MULT_RANGE_MODE0, X_SQ_DIST_OFFSET_RANGE_MODE0,
                  Y_SQ_DIST_OFFSET_RANGE_MODE0);
}

void CrystalBall::SetMode1RandomParams()
{
  SetRandomParams(X_AMPLITUDE_RANGE_MODE1, Y_AMPLITUDE_RANGE_MODE1, X_SQ_DIST_MULT_RANGE_MODE1,
                  Y_SQ_DIST_MULT_RANGE_MODE1, X_SQ_DIST_OFFSET_RANGE_MODE1,
                  Y_SQ_DIST_OFFSET_RANGE_MODE1);
}

void CrystalBall::SetRandomParams(const NumberRange<float>& xAmplitudeRange,
                                  const NumberRange<float>& yAmplitudeRange,
                                  const NumberRange<float>& xSqDistMultRange,
                                  const NumberRange<float>& ySqDistMultRange,
                                  const NumberRange<float>& xSqDistOffsetRange,
                                  const NumberRange<float>& ySqDistOffsetRange)
{
  m_params.xAmplitude = GetRandInRange(xAmplitudeRange);
  m_params.yAmplitude = ProbabilityOf(PROB_XY_AMPLITUDES_EQUAL) ? m_params.xAmplitude
                                                                : GetRandInRange(yAmplitudeRange);

  m_params.xSqDistMult = GetRandInRange(xSqDistMultRange);
  m_params.ySqDistMult = ProbabilityOf(PROB_XY_SQ_DIST_MULT_EQUAL)
                             ? m_params.xSqDistMult
                             : GetRandInRange(ySqDistMultRange);

  m_params.xSqDistOffset = GetRandInRange(xSqDistOffsetRange);
  m_params.ySqDistOffset = ProbabilityOf(PROB_XY_SQ_DIST_OFFSET_EQUAL)
                               ? m_params.xSqDistOffset
                               : GetRandInRange(ySqDistOffsetRange);
}

#if __cplusplus <= 201402L
} // namespace FILTERS
} // namespace GOOM
#else
} // namespace GOOM::FILTERS
#endif
