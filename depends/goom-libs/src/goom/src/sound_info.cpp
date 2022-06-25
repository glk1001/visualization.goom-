#include "sound_info.h"

#include "goom_config.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace GOOM
{

auto AudioSamples::GetSampleArrays(const std::vector<float>& floatAudioData)
    -> std::array<SampleArray, NUM_AUDIO_SAMPLES>
{
  Expects((NUM_AUDIO_SAMPLES * AUDIO_SAMPLE_LEN) == floatAudioData.size());

  std::array<SampleArray, NUM_AUDIO_SAMPLES> sampleArrays{};

  size_t fpos = 0;
  for (size_t i = 0; i < AUDIO_SAMPLE_LEN; ++i)
  {
    static_assert(sampleArrays.size() == 2);
    sampleArrays[0][i] = GetPositiveValue(floatAudioData[fpos]);

    ++fpos;

    sampleArrays[1][i] = GetPositiveValue(floatAudioData[fpos]);
    ++fpos;
  }

  return sampleArrays;
}

auto AudioSamples::GetMaxMinSampleValues(
    const std::array<SampleArray, NUM_AUDIO_SAMPLES>& sampleArrays)
    -> std::array<MaxMinValues, NUM_AUDIO_SAMPLES>
{
  std::array<MaxMinValues, NUM_AUDIO_SAMPLES> minMaxSampleValues{};

  for (size_t i = 0; i < NUM_AUDIO_SAMPLES; ++i)
  {
    const auto* const sampleArrayBegin = cbegin(sampleArrays.at(i));
    const auto* const sampleArrayEnd = cend(sampleArrays.at(i));
    const auto& [minVal, maxVal] = std::minmax_element(sampleArrayBegin, sampleArrayEnd);
    minMaxSampleValues.at(i).minVal = *minVal;
    minMaxSampleValues.at(i).maxVal = *maxVal;
  }

  return minMaxSampleValues;
}

AudioSamples::AudioSamples(const size_t numSampleChannels, const std::vector<float>& floatAudioData)
  : m_numDistinctChannels{numSampleChannels},
    m_sampleArrays{GetSampleArrays(floatAudioData)},
    m_minMaxSampleValues{GetMaxMinSampleValues(m_sampleArrays)},
    m_overallMinMaxSampleValues{
        std::min(m_minMaxSampleValues[0].minVal, m_minMaxSampleValues[1].minVal),
        std::max(m_minMaxSampleValues[0].maxVal, m_minMaxSampleValues[1].maxVal)}
{
  static_assert(2 == NUM_AUDIO_SAMPLES);
  Expects((0 < numSampleChannels) && (numSampleChannels <= 2));
}

static constexpr uint32_t NUM_GOOMS_IN_SHORT_CYCLE = 4;
static constexpr float GOOM_LIMIT_SHORT_CYCLE_INCREMENT = 0.02F;
static constexpr uint32_t NUM_GOOMS_IN_MEDIUM_CYCLE = 7;
static constexpr float GOOM_LIMIT_MEDIUM_CYCLE_INCREMENT = 0.03F;
static constexpr uint32_t NUM_GOOMS_IN_LONG_CYCLE = 16;
static constexpr float GOOM_LIMIT_LONG_CYCLE_INCREMENT = 0.04F;

void SoundInfo::ProcessSample(const AudioSamples& samples)
{
  ++m_updateNum;

  const float prevVolume = m_volume;
  UpdateVolume(samples);

  const float prevSpeed = m_speed;
  UpdateSpeed(prevVolume);

  UpdateAcceleration(prevSpeed);

  UpdateLastBigGoom();
  UpdateLastGoom();
}

inline void SoundInfo::UpdateVolume(const AudioSamples& samples)
{
  m_volume = samples.GetSampleOverallMinMax().maxVal;

  if (m_volume > m_allTimesMaxVolume)
  {
    m_allTimesMaxVolume = m_volume;
  }
  if (samples.GetSampleOverallMinMax().minVal < m_allTimesMinVolume)
  {
    m_allTimesMinVolume = samples.GetSampleOverallMinMax().minVal;
  }

  Ensures(0.0F <= m_volume && m_volume <= 1.0F);
}

inline void SoundInfo::UpdateSpeed(const float prevVolume)
{
  m_speed = AudioSamples::GetPositiveValue(m_volume - prevVolume);
  Ensures(0.0F <= m_speed && m_speed <= 1.0F);
}

inline void SoundInfo::UpdateAcceleration(const float prevSpeed)
{
  m_acceleration = AudioSamples::GetPositiveValue(m_speed - prevSpeed);
  Ensures(0.0F <= m_acceleration && m_acceleration <= 1.0F);

  CheckSettledGoomLimits();
}

void SoundInfo::UpdateLastGoom()
{
  // Temps du goom
  // Goom time

  ++m_timeSinceLastGoom;

  if (m_acceleration > m_goomLimit)
  {
    m_timeSinceLastGoom = 0;
    ++m_totalGoomsInCurrentCycle;
    m_goomPower = m_acceleration - m_goomLimit;
  }

  if (m_acceleration > m_maxAccelerationSinceLastReset)
  {
    m_maxAccelerationSinceLastReset = m_acceleration;
  }

  // Toute les 2 secondes: vérifier si le taux de goom est correct et le modifier sinon.
  // Every 2 seconds: check if the goom rate is correct and modify it otherwise.
  if (0 == (m_updateNum % CYCLE_TIME))
  {
    CheckGoomRate();
  }
}

void SoundInfo::CheckGoomRate()
{
  UpdateGoomLimit();
  Ensures((GOOM_LIMIT_MIN <= m_goomLimit) && (m_goomLimit <= GOOM_LIMIT_MAX));

  m_totalGoomsInCurrentCycle = 0;
  m_maxAccelerationSinceLastReset = 0.0F;
  m_bigGoomLimit = BIG_GOOM_FACTOR * m_goomLimit;
}

// Detection des nouveaux gross goom
// Detection of new big goom
void SoundInfo::UpdateLastBigGoom()
{
  ++m_timeSinceLastBigGoom;
  if (m_timeSinceLastBigGoom <= MAX_BIG_GOOM_DURATION)
  {
    return;
  }

  if ((m_speed > BIG_GOOM_SPEED_LIMIT) && (m_acceleration > m_bigGoomLimit))
  {
    m_timeSinceLastBigGoom = 0;
  }
}

inline void SoundInfo::CheckSettledGoomLimits()
{
  static constexpr uint32_t NUM_UPDATES_TO_SETTLE = 5;
  if (m_updateNum <= NUM_UPDATES_TO_SETTLE)
  {
    m_goomLimit = m_acceleration + GOOM_LIMIT_SHORT_CYCLE_INCREMENT;
    m_bigGoomLimit = BIG_GOOM_FACTOR * m_goomLimit;
  }
}

// Detection des nouveaux goom
// Detection of new goom
void SoundInfo::UpdateGoomLimit()
{
  static constexpr float VERY_SLOW_SPEED = 0.01F;
  static constexpr float GOOM_LIMIT_SLOW_SPEED_FACTOR = 0.91F;
  if (m_speed < VERY_SLOW_SPEED)
  {
    m_goomLimit *= GOOM_LIMIT_SLOW_SPEED_FACTOR;
  }

  if (m_totalGoomsInCurrentCycle > NUM_GOOMS_IN_SHORT_CYCLE)
  {
    m_goomLimit += GOOM_LIMIT_SHORT_CYCLE_INCREMENT;
  }
  else if (m_totalGoomsInCurrentCycle > NUM_GOOMS_IN_MEDIUM_CYCLE)
  {
    m_goomLimit *= 1.0F + GOOM_LIMIT_MEDIUM_CYCLE_INCREMENT;
    m_goomLimit += GOOM_LIMIT_MEDIUM_CYCLE_INCREMENT;
  }
  else if (m_totalGoomsInCurrentCycle > NUM_GOOMS_IN_LONG_CYCLE)
  {
    m_goomLimit *= 1.0F + GOOM_LIMIT_LONG_CYCLE_INCREMENT;
    m_goomLimit += GOOM_LIMIT_LONG_CYCLE_INCREMENT;
  }
  else if (0 == m_totalGoomsInCurrentCycle)
  {
    static constexpr float GOOM_LIMIT_ACCELERATION_DECREMENT = 0.03F;
    m_goomLimit = m_maxAccelerationSinceLastReset - GOOM_LIMIT_ACCELERATION_DECREMENT;
  }

  static constexpr float GOOM_LIMIT_TOO_BIG = 0.02F;
  static constexpr float GOOM_LIMIT_TOO_BIG_DECREMENT = 0.02F;
  if ((1 == m_totalGoomsInCurrentCycle) && (m_goomLimit > GOOM_LIMIT_TOO_BIG))
  {
    m_goomLimit -= GOOM_LIMIT_TOO_BIG_DECREMENT;
  }

  m_goomLimit = std::clamp(m_goomLimit, GOOM_LIMIT_MIN, GOOM_LIMIT_MAX);
}

} // namespace GOOM
