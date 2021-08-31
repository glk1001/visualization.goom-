#include "filter_wave.h"

#include "goomutils/enumutils.h"
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
using UTILS::NUM;
using UTILS::NumberRange;
using UTILS::ProbabilityOf;

constexpr Wave::WaveEffect DEFAULT_WAVE_EFFECT = Wave::WaveEffect::WAVE_SIN_EFFECT;

constexpr float DEFAULT_PERIODIC_FACTOR = 1.0F;
constexpr float DEFAULT_SIN_COS_PERIODIC_FACTOR = 0.5F;
constexpr NumberRange<float> PERIODIC_FACTOR_RANGE = {0.5F, 1.0F};
constexpr NumberRange<float> SIN_COS_PERIODIC_FACTOR_RANGE = {0.1F, 0.9F};

constexpr float DEFAULT_FREQ_FACTOR = 20.0F;
constexpr NumberRange<float> FREQ_FACTOR_RANGE = {1.0F, 50.0F};

constexpr float DEFAULT_AMPLITUDE = 0.01F;
constexpr NumberRange<float> AMPLITUDE_RANGE = {0.001F, 0.25F};

// These give weird but interesting wave results
constexpr NumberRange<float> SMALL_FREQ_FACTOR_RANGE = {0.001F, 0.1F};
constexpr NumberRange<float> BIG_AMPLITUDE_RANGE = {1.0F, 50.0F};

constexpr float PROB_ALLOW_STRANGE_WAVE_VALUES = 0.1F;
constexpr float PROB_WAVE_XY_EFFECTS_EQUAL = 0.75F;
constexpr float PROB_NO_PERIODIC_FACTOR = 0.2F;

Wave::Wave(const Modes mode) noexcept
  : m_mode{mode},
    m_params{DEFAULT_WAVE_EFFECT, DEFAULT_WAVE_EFFECT, DEFAULT_FREQ_FACTOR, DEFAULT_AMPLITUDE,
             DEFAULT_PERIODIC_FACTOR}
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
  if (ProbabilityOf(PROB_ALLOW_STRANGE_WAVE_VALUES))
  {
    SetWaveModeSettings(SMALL_FREQ_FACTOR_RANGE, BIG_AMPLITUDE_RANGE);
  }
  else
  {
    SetWaveModeSettings(FREQ_FACTOR_RANGE, AMPLITUDE_RANGE);
  }
}

void Wave::SetWaveModeSettings(const NumberRange<float>& freqFactorRange,
                               const NumberRange<float>& amplitudeRange)
{
  m_params.xWaveEffect = static_cast<WaveEffect>(GetRandInRange(0U, NUM<WaveEffect>));
  m_params.yWaveEffect = ProbabilityOf(PROB_WAVE_XY_EFFECTS_EQUAL)
                             ? m_params.xWaveEffect
                             : static_cast<WaveEffect>(GetRandInRange(0U, NUM<WaveEffect>));

  if (ProbabilityOf(PROB_NO_PERIODIC_FACTOR))
  {
    m_params.periodicFactor = m_params.xWaveEffect == WaveEffect::WAVE_SIN_COS_EFFECT
                                  ? DEFAULT_SIN_COS_PERIODIC_FACTOR
                                  : DEFAULT_PERIODIC_FACTOR;
  }
  else
  {
    m_params.periodicFactor = GetRandInRange(m_params.xWaveEffect == WaveEffect::WAVE_SIN_COS_EFFECT
                                                 ? SIN_COS_PERIODIC_FACTOR_RANGE
                                                 : PERIODIC_FACTOR_RANGE);
  }
  m_params.freqFactor = GetRandInRange(freqFactorRange);
  m_params.amplitude = GetRandInRange(amplitudeRange);
}

#if __cplusplus <= 201402L
} // namespace FILTERS
} // namespace GOOM
#else
} // namespace GOOM::FILTERS
#endif
