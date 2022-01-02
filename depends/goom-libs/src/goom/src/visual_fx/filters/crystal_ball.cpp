#include "crystal_ball.h"

#include "utils/name_value_pairs.h"

namespace GOOM::VISUAL_FX::FILTERS
{

using UTILS::IGoomRand;
using UTILS::NameValuePairs;

constexpr float DEFAULT_AMPLITUDE = 0.1F;
constexpr IGoomRand::NumberRange<float> X_AMPLITUDE_RANGE_MODE0 = {0.001F, 0.501F};
constexpr IGoomRand::NumberRange<float> Y_AMPLITUDE_RANGE_MODE0 = {0.001F, 0.501F};
constexpr IGoomRand::NumberRange<float> X_AMPLITUDE_RANGE_MODE1 = {0.500F, 1.001F};
constexpr IGoomRand::NumberRange<float> Y_AMPLITUDE_RANGE_MODE1 = {0.500F, 1.001F};

constexpr float DEFAULT_SQ_DIST_MULT = 0.025F;
constexpr IGoomRand::NumberRange<float> X_SQ_DIST_MULT_RANGE_MODE0 = {0.001F, 0.051F};
constexpr IGoomRand::NumberRange<float> Y_SQ_DIST_MULT_RANGE_MODE0 = {0.001F, 0.051F};
constexpr IGoomRand::NumberRange<float> X_SQ_DIST_MULT_RANGE_MODE1 = {0.050F, 0.101F};
constexpr IGoomRand::NumberRange<float> Y_SQ_DIST_MULT_RANGE_MODE1 = {0.050F, 0.101F};

constexpr float DEFAULT_SQ_DIST_OFFSET = 0.05F;
constexpr IGoomRand::NumberRange<float> X_SQ_DIST_OFFSET_RANGE_MODE0 = {0.001F, 0.11F};
constexpr IGoomRand::NumberRange<float> Y_SQ_DIST_OFFSET_RANGE_MODE0 = {0.001F, 0.11F};
constexpr IGoomRand::NumberRange<float> X_SQ_DIST_OFFSET_RANGE_MODE1 = {0.100F, 1.01F};
constexpr IGoomRand::NumberRange<float> Y_SQ_DIST_OFFSET_RANGE_MODE1 = {0.100F, 1.01F};

constexpr float PROB_XY_AMPLITUDES_EQUAL = 1.00F;
constexpr float PROB_XY_SQ_DIST_MULT_EQUAL = 1.00F;
constexpr float PROB_XY_SQ_DIST_OFFSET_EQUAL = 1.00F;

CrystalBall::CrystalBall(const Modes mode, const UTILS::IGoomRand& goomRand) noexcept
  : m_mode{mode}, m_goomRand{goomRand}, m_params{DEFAULT_AMPLITUDE,      DEFAULT_AMPLITUDE,
                                                 DEFAULT_SQ_DIST_MULT,   DEFAULT_SQ_DIST_MULT,
                                                 DEFAULT_SQ_DIST_OFFSET, DEFAULT_SQ_DIST_OFFSET}
{
}

void CrystalBall::SetRandomParams()
{
  if (m_mode == Modes::MODE0)
  {
    SetMode0RandomParams();
  }
  else
  {
    SetMode1RandomParams();
  }
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

void CrystalBall::SetRandomParams(const IGoomRand::NumberRange<float>& xAmplitudeRange,
                                  const IGoomRand::NumberRange<float>& yAmplitudeRange,
                                  const IGoomRand::NumberRange<float>& xSqDistMultRange,
                                  const IGoomRand::NumberRange<float>& ySqDistMultRange,
                                  const IGoomRand::NumberRange<float>& xSqDistOffsetRange,
                                  const IGoomRand::NumberRange<float>& ySqDistOffsetRange)
{
  const float xAmplitude = m_goomRand.GetRandInRange(xAmplitudeRange);
  const float yAmplitude = m_goomRand.ProbabilityOf(PROB_XY_AMPLITUDES_EQUAL)
                               ? xAmplitude
                               : m_goomRand.GetRandInRange(yAmplitudeRange);

  const float xSqDistMult = m_goomRand.GetRandInRange(xSqDistMultRange);
  const float ySqDistMult = m_goomRand.ProbabilityOf(PROB_XY_SQ_DIST_MULT_EQUAL)
                                ? xSqDistMult
                                : m_goomRand.GetRandInRange(ySqDistMultRange);

  const float xSqDistOffset = m_goomRand.GetRandInRange(xSqDistOffsetRange);
  const float ySqDistOffset = m_goomRand.ProbabilityOf(PROB_XY_SQ_DIST_OFFSET_EQUAL)
                                  ? xSqDistOffset
                                  : m_goomRand.GetRandInRange(ySqDistOffsetRange);

  SetParams({xAmplitude, yAmplitude, xSqDistMult, ySqDistMult, xSqDistOffset, ySqDistOffset});
}

auto CrystalBall::GetSpeedCoefficientsEffectNameValueParams() const -> NameValuePairs
{
  return NameValuePairs();
}

} // namespace GOOM::VISUAL_FX::FILTERS
