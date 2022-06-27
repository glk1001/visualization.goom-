#include "sound_info.h"

#include "goom_config.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <vector>

namespace GOOM
{

auto AudioSamples::GetSampleArrays(const std::vector<float>& floatAudioData)
    -> std::array<SampleArray, NUM_AUDIO_SAMPLES>
{
  Expects((NUM_AUDIO_SAMPLES * AUDIO_SAMPLE_LEN) == floatAudioData.size());

  std::array<SampleArray, NUM_AUDIO_SAMPLES> sampleArrays{};

  size_t fpos = 0;
  for (size_t j = 0; j < AUDIO_SAMPLE_LEN; ++j)
  {
    for (size_t i = 0; i < NUM_AUDIO_SAMPLES; ++i)
    {
      sampleArrays[i][j] = GetPositiveValue(floatAudioData[fpos]);
      ++fpos;
    }
  }

  return sampleArrays;
}

auto AudioSamples::GetMinMaxSampleValues(
    const std::array<SampleArray, NUM_AUDIO_SAMPLES>& sampleArrays)
    -> std::array<MinMaxValues, NUM_AUDIO_SAMPLES>
{
  std::array<MinMaxValues, NUM_AUDIO_SAMPLES> minMaxSampleValues{};

  for (size_t i = 0; i < NUM_AUDIO_SAMPLES; ++i)
  {
    const SampleArray& sampleArray = sampleArrays.at(i);
    const auto& [minVal, maxVal] = std::minmax_element(cbegin(sampleArray), cend(sampleArray));
    minMaxSampleValues.at(i).minVal = *minVal;
    minMaxSampleValues.at(i).maxVal = *maxVal;
  }

  return minMaxSampleValues;
}

AudioSamples::AudioSamples(const size_t numSampleChannels, const std::vector<float>& floatAudioData)
  : m_numDistinctChannels{numSampleChannels},
    m_sampleArrays{GetSampleArrays(floatAudioData)},
    m_minMaxSampleValues{GetMinMaxSampleValues(m_sampleArrays)},
    m_overallMinMaxSampleValues{
        std::min(m_minMaxSampleValues[0].minVal, m_minMaxSampleValues[1].minVal),
        std::max(m_minMaxSampleValues[0].maxVal, m_minMaxSampleValues[1].maxVal)}
{
  static_assert(2 == NUM_AUDIO_SAMPLES);
  Expects((0 < numSampleChannels) && (numSampleChannels <= 2));
}

void SoundInfo::ProcessSample(const AudioSamples& samples)
{
  ++m_updateNum;

  const float prevVolume = m_volume;
  UpdateVolume(samples);

  const float prevSpeed = m_speed;
  UpdateSpeed(prevVolume);

  UpdateAcceleration(prevSpeed);
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
}

} // namespace GOOM
