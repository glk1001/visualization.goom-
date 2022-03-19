#include "wave.h"

#include "utils/enumutils.h"
#include "utils/name_value_pairs.h"

namespace GOOM::VISUAL_FX::FILTERS
{

using UTILS::NameValuePairs;
using UTILS::MATH::IGoomRand;

static constexpr Wave::WaveEffect DEFAULT_WAVE_EFFECT = Wave::WaveEffect::WAVE_SIN_EFFECT;

static constexpr float DEFAULT_PERIODIC_FACTOR = 1.0F;
static constexpr float DEFAULT_SIN_COS_PERIODIC_FACTOR = 0.5F;
static constexpr IGoomRand::NumberRange<float> PERIODIC_FACTOR_RANGE = {0.5F, 1.0F};
static constexpr IGoomRand::NumberRange<float> SIN_COS_PERIODIC_FACTOR_RANGE = {0.1F, 0.9F};

static constexpr float DEFAULT_FREQ_FACTOR = 20.0F;
static constexpr IGoomRand::NumberRange<float> FREQ_FACTOR_RANGE = {1.0F, 50.0F};

static constexpr float DEFAULT_AMPLITUDE = 0.01F;
static constexpr IGoomRand::NumberRange<float> AMPLITUDE_RANGE = {0.001F, 0.25F};

static constexpr float DEFAULT_REDUCER_COEFF = 1.0F;
static constexpr IGoomRand::NumberRange<float> REDUCER_COEFF_RANGE = {0.95F, 1.5F};
static constexpr IGoomRand::NumberRange<float> TAN_REDUCER_COEFF_RANGE = {4.0F, 10.0F};

// These give weird but interesting wave results
static constexpr IGoomRand::NumberRange<float> SMALL_FREQ_FACTOR_RANGE = {0.001F, 0.1F};
static constexpr IGoomRand::NumberRange<float> BIG_AMPLITUDE_RANGE = {1.0F, 50.0F};

static constexpr float PROB_ALLOW_STRANGE_WAVE_VALUES = 0.1F;
static constexpr float PROB_WAVE_XY_EFFECTS_EQUAL = 0.75F;
static constexpr float PROB_NO_PERIODIC_FACTOR = 0.2F;
static constexpr float PROB_PERIODIC_FACTOR_USES_X_WAVE_EFFECT = 0.9F;

// clang-format off
static constexpr float WAVE_SIN_EFFECT_WEIGHT      =  200.0F;
static constexpr float WAVE_COS_EFFECT_WEIGHT      =  200.0F;
static constexpr float WAVE_SIN_COS_EFFECT_WEIGHT  =  150.0F;
static constexpr float WAVE_TAN_EFFECT_WEIGHT      =  10.0F;
static constexpr float WAVE_TAN_SIN_EFFECT_WEIGHT  =  10.0F;
static constexpr float CWAVE_TAN_COS_EFFECT_WEIGHT =  10.0F;
static constexpr float WAVE_COT_EFFECT_WEIGHT      =   1.0F;
static constexpr float WAVE_COT_SIN_EFFECT_WEIGHT  =   1.0F;
static constexpr float WAVE_COT_COS_EFFECT_WEIGHT  =   1.0F;
// clang-format on


Wave::Wave(const Modes mode, const IGoomRand& goomRand)
  : m_mode{mode},
    m_goomRand{goomRand},
    m_weightedEffects{
      goomRand,
      {
        {    WaveEffect::WAVE_SIN_EFFECT,      WAVE_SIN_EFFECT_WEIGHT},
        {    WaveEffect::WAVE_COS_EFFECT,      WAVE_COS_EFFECT_WEIGHT},
        {WaveEffect::WAVE_SIN_COS_EFFECT,  WAVE_SIN_COS_EFFECT_WEIGHT},
        {    WaveEffect::WAVE_TAN_EFFECT,      WAVE_TAN_EFFECT_WEIGHT},
        {WaveEffect::WAVE_TAN_SIN_EFFECT,  WAVE_TAN_SIN_EFFECT_WEIGHT},
        {WaveEffect::WAVE_TAN_COS_EFFECT, CWAVE_TAN_COS_EFFECT_WEIGHT},
        {    WaveEffect::WAVE_COT_EFFECT,      WAVE_COT_EFFECT_WEIGHT},
        {WaveEffect::WAVE_COT_SIN_EFFECT,  WAVE_COT_SIN_EFFECT_WEIGHT},
        {WaveEffect::WAVE_COT_COS_EFFECT,  WAVE_COT_COS_EFFECT_WEIGHT},
      }
    },
    m_params{DEFAULT_WAVE_EFFECT, DEFAULT_WAVE_EFFECT,     DEFAULT_FREQ_FACTOR,
             DEFAULT_AMPLITUDE,   DEFAULT_PERIODIC_FACTOR, DEFAULT_REDUCER_COEFF}
{
}

void Wave::SetRandomParams()
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

void Wave::SetMode0RandomParams()
{
  SetWaveModeSettings(FREQ_FACTOR_RANGE, AMPLITUDE_RANGE);
}

void Wave::SetMode1RandomParams()
{
  if (m_goomRand.ProbabilityOf(PROB_ALLOW_STRANGE_WAVE_VALUES))
  {
    SetWaveModeSettings(SMALL_FREQ_FACTOR_RANGE, BIG_AMPLITUDE_RANGE);
  }
  else
  {
    SetWaveModeSettings(FREQ_FACTOR_RANGE, AMPLITUDE_RANGE);
  }
}

void Wave::SetWaveModeSettings(const IGoomRand::NumberRange<float>& freqFactorRange,
                               const IGoomRand::NumberRange<float>& amplitudeRange)
{
  const bool waveEffectsEqual = m_goomRand.ProbabilityOf(PROB_WAVE_XY_EFFECTS_EQUAL);

  const auto xWaveEffect = m_weightedEffects.GetRandomWeighted();
  const WaveEffect yWaveEffect =
      waveEffectsEqual ? xWaveEffect : m_weightedEffects.GetRandomWeighted();

  const float periodicFactor = GetPeriodicFactor(xWaveEffect, yWaveEffect);
  const float freqFactor = m_goomRand.GetRandInRange(freqFactorRange);
  const float amplitude = m_goomRand.GetRandInRange(amplitudeRange);
  const float reducerCoeff = GetReducerCoeff(xWaveEffect, yWaveEffect, periodicFactor);

  SetParams({xWaveEffect, yWaveEffect, freqFactor, amplitude, periodicFactor, reducerCoeff});
}

inline auto Wave::GetReducerCoeff(const WaveEffect xWaveEffect,
                                  [[maybe_unused]] const WaveEffect yWaveEffect,
                                  const float periodicFactor) const -> float
{
  switch (xWaveEffect)
  {
    case WaveEffect::WAVE_SIN_EFFECT:
    case WaveEffect::WAVE_COS_EFFECT:
    case WaveEffect::WAVE_SIN_COS_EFFECT:
      return m_goomRand.GetRandInRange(REDUCER_COEFF_RANGE);
    case WaveEffect::WAVE_TAN_EFFECT:
    case WaveEffect::WAVE_COT_EFFECT:
      return m_goomRand.GetRandInRange(TAN_REDUCER_COEFF_RANGE);
    case WaveEffect::WAVE_TAN_SIN_EFFECT:
    case WaveEffect::WAVE_TAN_COS_EFFECT:
    case WaveEffect::WAVE_COT_SIN_EFFECT:
    case WaveEffect::WAVE_COT_COS_EFFECT:
      return STD20::lerp(m_goomRand.GetRandInRange(TAN_REDUCER_COEFF_RANGE),
                         m_goomRand.GetRandInRange(REDUCER_COEFF_RANGE), periodicFactor);
    default:
      throw std::logic_error("Unknown WaveEffect enum");
  }
}

inline auto Wave::GetPeriodicFactor(const WaveEffect xWaveEffect,
                                    const WaveEffect yWaveEffect) const -> float
{
  if (m_goomRand.ProbabilityOf(PROB_NO_PERIODIC_FACTOR))
  {
    return xWaveEffect == WaveEffect::WAVE_SIN_COS_EFFECT ? DEFAULT_SIN_COS_PERIODIC_FACTOR
                                                          : DEFAULT_PERIODIC_FACTOR;
  }
  if (m_goomRand.ProbabilityOf(PROB_PERIODIC_FACTOR_USES_X_WAVE_EFFECT))
  {
    return m_goomRand.GetRandInRange(xWaveEffect == WaveEffect::WAVE_SIN_COS_EFFECT
                                         ? SIN_COS_PERIODIC_FACTOR_RANGE
                                         : PERIODIC_FACTOR_RANGE);
  }

  return m_goomRand.GetRandInRange(yWaveEffect == WaveEffect::WAVE_SIN_COS_EFFECT
                                       ? SIN_COS_PERIODIC_FACTOR_RANGE
                                       : PERIODIC_FACTOR_RANGE);
}

auto Wave::GetSpeedCoefficientsEffectNameValueParams() const -> NameValuePairs
{
  return NameValuePairs();
}

} // namespace GOOM::VISUAL_FX::FILTERS
