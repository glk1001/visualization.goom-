#include "sound_info.h"

#include "goom_config.h"
#include "utils/mathutils.h"

#include <algorithm>
#undef NDEBUG
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <format>
#include <limits>
#include <stdexcept>
#include <vector>

namespace GOOM
{

const size_t AudioSamples::NUM_AUDIO_SAMPLES = 2;
const size_t AudioSamples::AUDIO_SAMPLE_LEN = 512;

inline auto AudioSamples::FloatToInt16(const float fVal) -> int16_t
{
  if (fVal >= 1.0F)
  {
    return std::numeric_limits<int16_t>::max();
  }

  if (fVal < -1.0F)
  {
    return -std::numeric_limits<int16_t>::max();
  }

  return static_cast<int16_t>(fVal * static_cast<float>(std::numeric_limits<int16_t>::max()));
}

auto AudioSamples::GetSampleArrays(const std::vector<float>& floatAudioData)
    -> std::vector<SampleArray>
{
  assert((NUM_AUDIO_SAMPLES * AUDIO_SAMPLE_LEN) == floatAudioData.size());

  std::vector<SampleArray> sampleArrays(NUM_AUDIO_SAMPLES);

  sampleArrays[0].resize(AUDIO_SAMPLE_LEN);
  sampleArrays[1].resize(AUDIO_SAMPLE_LEN);

  if (1 == NUM_AUDIO_SAMPLES)
  {
    for (size_t i = 0; i < AUDIO_SAMPLE_LEN; ++i)
    {
      sampleArrays[0][i] = FloatToInt16(floatAudioData[i]);
      sampleArrays[1][i] = sampleArrays[0][i];
    }
  }
  else
  {
    size_t fpos = 0;
    for (size_t i = 0; i < AUDIO_SAMPLE_LEN; ++i)
    {
      sampleArrays[0][i] = FloatToInt16(floatAudioData[fpos]);
      ++fpos;

      sampleArrays[1][i] = FloatToInt16(floatAudioData[fpos]);
      ++fpos;
    }
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
    m_minMaxSampleValues{GetMaxMinSampleValues(m_sampleArrays)}
{
  assert((0 < numSampleChannels) && (numSampleChannels <= 2));
}

void SoundInfo::ProcessSample(const AudioSamples& samples)
{
  ++m_updateNum;

  UpdateVolume(samples);
  assert(0.0F <= m_volume && m_volume <= 1.0F);

  const float prevAcceleration = m_acceleration;
  UpdateAcceleration();
  assert(0.0F <= m_acceleration && m_acceleration <= 1.0F);

  UpdateSpeed(prevAcceleration);
  assert(0.0F <= m_speed && m_speed <= 1.0F);

  // Detection des nouveaux gooms
  // Detection of new gooms
  UpdateLastBigGoom();
  UpdateLastGoom();
}

void SoundInfo::UpdateVolume(const AudioSamples& samples)
{
  // Find the min/max of volumes
  int16_t maxPosVar = 0;
  int16_t maxVar = std::numeric_limits<int16_t>::min();
  int16_t minVar = std::numeric_limits<int16_t>::max();
  for (size_t i = 0; i < AudioSamples::NUM_AUDIO_SAMPLES; ++i)
  {
    const std::vector<int16_t>& soundData = samples.GetSample(i);
    for (const int16_t dataVal : soundData)
    {
      if (maxPosVar < dataVal)
      {
        maxPosVar = dataVal;
      }
      if (maxVar < dataVal)
      {
        maxVar = dataVal;
      }
      if (minVar > dataVal)
      {
        minVar = dataVal;
      }
    }
  }

  if (maxPosVar > m_allTimesPositiveMaxVolume)
  {
    m_allTimesPositiveMaxVolume = maxPosVar;
  }
  if (maxVar > m_allTimesMaxVolume)
  {
    m_allTimesMaxVolume = maxVar;
  }
  if (minVar < m_allTimesMinVolume)
  {
    m_allTimesMinVolume = minVar;
  }

  // Volume sonore - TODO: why only positive volumes?
  if (m_allTimesPositiveMaxVolume > 0)
  {
    m_volume = static_cast<float>(maxPosVar) / static_cast<float>(m_allTimesPositiveMaxVolume);
  }
}

void SoundInfo::UpdateAcceleration()
{
  // Transformations sur la vitesse du son
  // Speed of sound transformations
  m_acceleration = m_volume;

  constexpr float VERY_SLOW_SPEED = 0.1F;
  constexpr float SLOW_SPEED = 0.3F;

  if (m_speed < VERY_SLOW_SPEED)
  {
    m_acceleration *= (1.0F - m_speed);
  }
  else if (m_speed < SLOW_SPEED)
  {
    m_acceleration *= (0.9F - (0.5F * (m_speed - VERY_SLOW_SPEED)));
  }
  else
  {
    m_acceleration *= (0.8F - (0.25F * (m_speed - SLOW_SPEED)));
  }

  // Adoucissement de l'acceleration
  // Smooth acceleration
  if (m_acceleration < 0.0F)
  {
    m_acceleration = 0.0F;
  }
  else
  {
    m_acceleration *= ACCELERATION_MULTIPLIER;
  }
}

void SoundInfo::UpdateSpeed(const float prevAcceleration)
{
  // Mise a jour de la vitesse
  // Speed update
  float diffAcceleration = m_acceleration - prevAcceleration;
  if (diffAcceleration < 0.0F)
  {
    diffAcceleration = -diffAcceleration;
  }

  constexpr float LERP_SPEED_MIX = 0.75;
  const float newSpeed = SPEED_MULTIPLIER * ((0.5F * m_speed) + (0.25F * diffAcceleration));
  m_speed = stdnew::lerp(newSpeed, m_speed, LERP_SPEED_MIX);
  m_speed = stdnew::clamp(m_speed, 0.0F, 1.0F);
}

void SoundInfo::UpdateLastGoom()
{
  // Temps du goom
  // Goom time
  ++m_timeSinceLastGoom;

  if (m_acceleration > m_goomLimit)
  {
    m_timeSinceLastGoom = 0;

    // TODO: tester && (info->m_timeSinceLastGoom > 20)) {
    ++m_totalGoomsInCurrentCycle;
    m_goomPower = m_acceleration - m_goomLimit;
  }
  if (m_acceleration > m_maxAccelSinceLastReset)
  {
    m_maxAccelSinceLastReset = m_acceleration;
  }

  // Toute les 2 secondes: vérifier si le taux de goom est correct et le modifier sinon.
  // Every 2 seconds: check if the goom rate is correct and modify it otherwise.
  if (0 == (m_updateNum % CYCLE_TIME))
  {
    UpdateGoomLimit();
    assert((0.0F <= m_goomLimit) && (m_goomLimit <= 1.0F));

    m_totalGoomsInCurrentCycle = 0;
    m_maxAccelSinceLastReset = 0.0F;
    m_bigGoomLimit = m_goomLimit * BIG_GOOM_FACTOR;
  }

  // m_bigGoomLimit == m_goomLimit*9/8+7 ?
}

void SoundInfo::UpdateLastBigGoom()
{
  ++m_timeSinceLastBigGoom;

  if ((m_speed > BIG_GOOM_SPEED_LIMIT) && (m_acceleration > m_bigGoomLimit) &&
      (m_timeSinceLastBigGoom > MAX_BIG_GOOM_DURATION))
  {
    m_timeSinceLastBigGoom = 0;
  }
}

void SoundInfo::UpdateGoomLimit()
{
  constexpr float VERY_SLOW_SPEED = 0.01F;
  constexpr float VERY_SLOW_SPEED_FACTOR = 0.91F;
  if (m_speed < VERY_SLOW_SPEED)
  {
    m_goomLimit *= VERY_SLOW_SPEED_FACTOR;
  }

  constexpr uint32_t TOTAL_GOOMS_SHORT_CYCLE = 4;
  constexpr float TOTAL_GOOMS_SHORT_INCREMENT = 0.02F;
  constexpr uint32_t TOTAL_GOOMS_MEDIUM_CYCLE = 7;
  constexpr float TOTAL_GOOMS_MEDIUM_INCREMENT = 0.03F;
  constexpr uint32_t TOTAL_GOOMS_BIG_CYCLE = 16;
  constexpr float TOTAL_GOOMS_BIG_INCREMENT = 0.04F;
  constexpr float ACCEL_DECREMENT = 0.02F;

  if (m_totalGoomsInCurrentCycle > TOTAL_GOOMS_SHORT_CYCLE)
  {
    m_goomLimit += TOTAL_GOOMS_SHORT_INCREMENT;
  }
  else if (m_totalGoomsInCurrentCycle > TOTAL_GOOMS_MEDIUM_CYCLE)
  {
    m_goomLimit *= 1.0F + TOTAL_GOOMS_MEDIUM_INCREMENT;
    m_goomLimit += TOTAL_GOOMS_MEDIUM_INCREMENT;
  }
  else if (m_totalGoomsInCurrentCycle > TOTAL_GOOMS_BIG_CYCLE)
  {
    m_goomLimit *= 1.0F + TOTAL_GOOMS_BIG_INCREMENT;
    m_goomLimit += TOTAL_GOOMS_BIG_INCREMENT;
  }
  else if (0 == m_totalGoomsInCurrentCycle)
  {
    m_goomLimit = m_maxAccelSinceLastReset - ACCEL_DECREMENT;
  }

  constexpr float TOO_BIG_LIMIT = 0.02F;
  constexpr float SMALL_DECREMENT = 0.01F;
  if ((1 == m_totalGoomsInCurrentCycle) && (m_goomLimit > TOO_BIG_LIMIT))
  {
    m_goomLimit -= SMALL_DECREMENT;
  }

  constexpr float MIN_LIMIT = 0.0F;
  constexpr float MAX_LIMIT = 1.0F;
  m_goomLimit = stdnew::clamp(m_goomLimit, MIN_LIMIT, MAX_LIMIT);
}

} // namespace GOOM
