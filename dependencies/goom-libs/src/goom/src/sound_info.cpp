#undef NDEBUG
#include "sound_info.h"

#include "utils/mathutils.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace GOOM
{

const size_t AudioSamples::NUM_AUDIO_SAMPLES = 2;
const size_t AudioSamples::AUDIO_SAMPLE_LEN = 512;

auto AudioSamples::GetSampleArrays(const std::vector<float>& floatAudioData)
    -> std::vector<SampleArray>
{
  assert((NUM_AUDIO_SAMPLES * AUDIO_SAMPLE_LEN) == floatAudioData.size());

  std::vector<SampleArray> sampleArrays(NUM_AUDIO_SAMPLES);

  sampleArrays[0].resize(AUDIO_SAMPLE_LEN);
  sampleArrays[1].resize(AUDIO_SAMPLE_LEN);

    size_t fpos = 0;
    for (size_t i = 0; i < AUDIO_SAMPLE_LEN; ++i)
    {
      sampleArrays[0][i] = GetPositiveValue(floatAudioData[fpos]);
      if (NUM_AUDIO_SAMPLES != 1)
      {
        ++fpos;
      }

      sampleArrays[1][i] = GetPositiveValue(floatAudioData[fpos]);
      ++fpos;
    }

  return sampleArrays;
}

inline auto AudioSamples::GetMaxMinSampleValues(const std::vector<SampleArray>& sampleArrays)
    -> std::vector<MaxMinValues>
{
  std::vector<MaxMinValues> minMaxSampleValues(NUM_AUDIO_SAMPLES);

  for (size_t i = 0; i < minMaxSampleValues.size(); ++i)
  {
    const auto sampleArrayBegin = cbegin(sampleArrays.at(i));
    const auto sampleArrayEnd = cend(sampleArrays.at(i));
    minMaxSampleValues[i].minVal = *std::min_element(sampleArrayBegin, sampleArrayEnd);
    minMaxSampleValues[i].maxVal = *std::max_element(sampleArrayBegin, sampleArrayEnd);
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
  assert((0 < numSampleChannels) && (numSampleChannels <= 2));
}

constexpr uint32_t NUM_GOOMS_IN_SHORT_CYCLE = 4;
constexpr float GOOM_LIMIT_SHORT_CYCLE_INCREMENT = 0.02F;
constexpr uint32_t NUM_GOOMS_IN_MEDIUM_CYCLE = 7;
constexpr float GOOM_LIMIT_MEDIUM_CYCLE_INCREMENT = 0.03F;
constexpr uint32_t NUM_GOOMS_IN_LONG_CYCLE = 16;
constexpr float GOOM_LIMIT_LONG_CYCLE_INCREMENT = 0.04F;

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

  assert(0.0F <= m_volume && m_volume <= 1.0F);
}

inline void SoundInfo::UpdateSpeed(const float prevVolume)
{
  m_speed = AudioSamples::GetPositiveValue(m_volume - prevVolume);
  assert(0.0F <= m_speed && m_speed <= 1.0F);
}

inline void SoundInfo::UpdateAcceleration(const float prevSpeed)
{
  m_acceleration = AudioSamples::GetPositiveValue(m_speed - prevSpeed);
  assert(0.0F <= m_acceleration && m_acceleration <= 1.0F);

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
  assert((GOOM_LIMIT_MIN <= m_goomLimit) && (m_goomLimit <= GOOM_LIMIT_MAX));

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
  constexpr uint32_t NUM_UPDATES_TO_SETTLE = 5;
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
  constexpr float VERY_SLOW_SPEED = 0.01F;
  constexpr float GOOM_LIMIT_SLOW_SPEED_FACTOR = 0.91F;
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
    constexpr float GOOM_LIMIT_ACCELERATION_DECREMENT = 0.03F;
    m_goomLimit = m_maxAccelerationSinceLastReset - GOOM_LIMIT_ACCELERATION_DECREMENT;
  }

  constexpr float GOOM_LIMIT_TOO_BIG = 0.02F;
  constexpr float GOOM_LIMIT_TOO_BIG_DECREMENT = 0.02F;
  if ((1 == m_totalGoomsInCurrentCycle) && (m_goomLimit > GOOM_LIMIT_TOO_BIG))
  {
    m_goomLimit -= GOOM_LIMIT_TOO_BIG_DECREMENT;
  }

  m_goomLimit = stdnew::clamp(m_goomLimit, GOOM_LIMIT_MIN, GOOM_LIMIT_MAX);
}

} // namespace GOOM
