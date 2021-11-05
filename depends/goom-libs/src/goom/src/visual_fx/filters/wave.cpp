#include "wave.h"

#include "utils/enumutils.h"
#include "utils/name_value_pairs.h"

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

using UTILS::IGoomRand;
using UTILS::NameValuePairs;
using UTILS::NUM;

constexpr Wave::WaveEffect DEFAULT_WAVE_EFFECT = Wave::WaveEffect::WAVE_SIN_EFFECT;

constexpr float DEFAULT_PERIODIC_FACTOR = 1.0F;
constexpr float DEFAULT_SIN_COS_PERIODIC_FACTOR = 0.5F;
constexpr IGoomRand::NumberRange<float> PERIODIC_FACTOR_RANGE = {0.5F, 1.0F};
constexpr IGoomRand::NumberRange<float> SIN_COS_PERIODIC_FACTOR_RANGE = {0.1F, 0.9F};

constexpr float DEFAULT_FREQ_FACTOR = 20.0F;
constexpr IGoomRand::NumberRange<float> FREQ_FACTOR_RANGE = {1.0F, 50.0F};

constexpr float DEFAULT_AMPLITUDE = 0.01F;
constexpr IGoomRand::NumberRange<float> AMPLITUDE_RANGE = {0.001F, 0.25F};

// These give weird but interesting wave results
constexpr IGoomRand::NumberRange<float> SMALL_FREQ_FACTOR_RANGE = {0.001F, 0.1F};
constexpr IGoomRand::NumberRange<float> BIG_AMPLITUDE_RANGE = {1.0F, 50.0F};

constexpr float PROB_ALLOW_STRANGE_WAVE_VALUES = 0.1F;
constexpr float PROB_WAVE_XY_EFFECTS_EQUAL = 0.75F;
constexpr float PROB_NO_PERIODIC_FACTOR = 0.2F;

Wave::Wave(const Modes mode, IGoomRand& goomRand) noexcept
  : m_mode{mode},
    m_goomRand{goomRand},
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
  m_params.xWaveEffect = static_cast<WaveEffect>(m_goomRand.GetRandInRange(0U, NUM<WaveEffect>));
  m_params.yWaveEffect =
      m_goomRand.ProbabilityOf(PROB_WAVE_XY_EFFECTS_EQUAL)
          ? m_params.xWaveEffect
          : static_cast<WaveEffect>(m_goomRand.GetRandInRange(0U, NUM<WaveEffect>));

  if (m_goomRand.ProbabilityOf(PROB_NO_PERIODIC_FACTOR))
  {
    m_params.periodicFactor = m_params.xWaveEffect == WaveEffect::WAVE_SIN_COS_EFFECT
                                  ? DEFAULT_SIN_COS_PERIODIC_FACTOR
                                  : DEFAULT_PERIODIC_FACTOR;
  }
  else
  {
    m_params.periodicFactor = m_goomRand.GetRandInRange(
        m_params.xWaveEffect == WaveEffect::WAVE_SIN_COS_EFFECT ? SIN_COS_PERIODIC_FACTOR_RANGE
                                                                : PERIODIC_FACTOR_RANGE);
  }
  m_params.freqFactor = m_goomRand.GetRandInRange(freqFactorRange);
  m_params.amplitude = m_goomRand.GetRandInRange(amplitudeRange);
}

auto Wave::GetSpeedCoefficientsEffectNameValueParams() const -> NameValuePairs
{
  return NameValuePairs();
}

#if __cplusplus <= 201402L
} // namespace FILTERS
} // namespace GOOM
#else
} // namespace GOOM::FILTERS
#endif
