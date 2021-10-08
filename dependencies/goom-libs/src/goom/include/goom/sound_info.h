#pragma once

#include "goom_config.h"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>

namespace GOOM
{

class AudioSamples
{
public:
  // GCC doesn't like this (link problem):
  //static constexpr size_t NUM_AUDIO_SAMPLES = 2;
  //static constexpr size_t AUDIO_SAMPLE_LEN = 512;
  static const size_t NUM_AUDIO_SAMPLES;
  static const size_t AUDIO_SAMPLE_LEN;
  [[nodiscard]] static constexpr auto GetPositiveValue(float audioValue) -> float;

  // AudioSample object: numSampleChannels = 1 or 2.
  //   If numSampleChannels = 1, then the first  AUDIO_SAMPLE_LEN values of 'floatAudioData'
  //   are used to for two channels.
  //   If numSampleChannels = 2, then the 'floatAudioData' must interleave the two channels,
  //   one after the other. So 'floatAudioData[0]' is channel 0, 'floatAudioData[1]' is
  //   channel 1, 'floatAudioData[2]' is channel 0, 'floatAudioData[3]' is channel 1, etc.
  AudioSamples(size_t numSampleChannels,
               const std::vector<float>& floatAudioData);

  [[nodiscard]] auto GetNumDistinctChannels() const -> size_t;

  struct MaxMinValues
  {
    float minVal;
    float maxVal;
  };
  using SampleArray = std::vector<float>;
  [[nodiscard]] auto GetSample(size_t channelIndex) const -> const SampleArray&;
  [[nodiscard]] auto GetSampleMinMax(size_t channelIndex) const -> const MaxMinValues&;
  [[nodiscard]] auto GetSampleOverallMinMax() const -> const MaxMinValues&;

private:
  const size_t m_numDistinctChannels;
  const std::vector<SampleArray> m_sampleArrays;
  const std::vector<MaxMinValues> m_minMaxSampleValues;
  const MaxMinValues m_overallMinMaxSampleValues;
  [[nodiscard]] static auto GetSampleArrays(const std::vector<float>& floatAudioData)
      -> std::vector<SampleArray>;
  [[nodiscard]] static auto GetMaxMinSampleValues(const std::vector<SampleArray>& sampleArrays)
      -> std::vector<MaxMinValues>;
};

class SoundInfo
{
public:
  SoundInfo() noexcept = default;

  void ProcessSample(const AudioSamples& samples);

  // Note: a Goom is just a sound event...
  [[nodiscard]] auto GetTimeSinceLastGoom() const -> uint32_t; // >= 0
  [[nodiscard]] auto GetTimeSinceLastBigGoom() const -> uint32_t; // >= 0

  // Number of Gooms since last reset (every 'CYCLE_TIME')
  [[nodiscard]] auto GetTotalGoomsInCurrentCycle() const -> uint32_t;

  // Power of the last Goom [0..1]
  [[nodiscard]] auto GetGoomPower() const -> float;

  // Volume of the sound [0..1]
  [[nodiscard]] auto GetVolume() const -> float;

  // Speed of the sound [0..1]
  [[nodiscard]] auto GetSpeed() const -> float;

  // Acceleration of the sound [0..1]
  [[nodiscard]] auto GetAcceleration() const -> float;

  [[nodiscard]] auto GetAllTimesMaxVolume() const -> float;
  [[nodiscard]] auto GetAllTimesMinVolume() const -> float;

  // For debugging
  [[nodiscard]] auto GetGoomLimit() const -> float;
  [[nodiscard]] auto GetBigGoomLimit() const -> float;

private:
  uint32_t m_updateNum = 0;
  uint32_t m_totalGoomsInCurrentCycle = 0;
  static constexpr uint32_t CYCLE_TIME = 64;
  uint32_t m_timeSinceLastGoom = 0;
  static constexpr uint32_t MAX_BIG_GOOM_DURATION = 100;
  uint32_t m_timeSinceLastBigGoom = 0;
  float m_goomPower = 0.0F;

  static constexpr float GOOM_LIMIT_MIN = 0.0F;
  static constexpr float GOOM_LIMIT_MAX = 1.0F;
  float m_goomLimit = 1.0F; // auto-updated limit of goom_detection

  static constexpr float BIG_GOOM_SPEED_LIMIT = 0.10F;
  static constexpr float BIG_GOOM_FACTOR = 1.01F;
  float m_bigGoomLimit = 1.0F;

  float m_volume = 0.0F;
  float m_acceleration = 0.0F;
  float m_speed = 0.0F;

  float m_allTimesMaxVolume = std::numeric_limits<float>::min();
  float m_allTimesMinVolume = std::numeric_limits<float>::max();
  float m_maxAccelerationSinceLastReset = 0.0F;

  void UpdateVolume(const AudioSamples& samples);
  void UpdateSpeed(float prevVolume);
  void UpdateAcceleration(float prevSpeed);
  void UpdateLastGoom();
  void UpdateLastBigGoom();
  void UpdateGoomLimit();
  void CheckGoomRate();
  void CheckSettledGoomLimits();
};

constexpr auto AudioSamples::GetPositiveValue(const float audioValue) -> float
{
  assert(-1.0F <= audioValue);
  assert(audioValue <= 1.0F);

  return 0.5F * (1.0F + audioValue);
}

inline auto AudioSamples::GetNumDistinctChannels() const -> size_t
{
  return m_numDistinctChannels;
}

inline auto AudioSamples::GetSample(const size_t channelIndex) const -> const SampleArray&
{
  return m_sampleArrays.at(channelIndex);
}

inline auto AudioSamples::GetSampleMinMax(const size_t channelIndex) const -> const MaxMinValues&
{
  return m_minMaxSampleValues.at(channelIndex);
}

inline auto AudioSamples::GetSampleOverallMinMax() const -> const MaxMinValues&
{
  return m_overallMinMaxSampleValues;
}

inline auto SoundInfo::GetTotalGoomsInCurrentCycle() const -> uint32_t
{
  return m_totalGoomsInCurrentCycle;
}

inline auto SoundInfo::GetTimeSinceLastGoom() const -> uint32_t
{
  return m_timeSinceLastGoom;
}

inline auto SoundInfo::GetTimeSinceLastBigGoom() const -> uint32_t
{
  return m_timeSinceLastBigGoom;
}

inline auto SoundInfo::GetVolume() const -> float
{
  return m_volume;
}

inline auto SoundInfo::GetSpeed() const -> float
{
  return m_speed;
}

inline auto SoundInfo::GetAcceleration() const -> float
{
  return m_acceleration;
}

inline auto SoundInfo::GetAllTimesMaxVolume() const -> float
{
  return m_allTimesMaxVolume;
}

inline auto SoundInfo::GetAllTimesMinVolume() const -> float
{
  return m_allTimesMinVolume;
}

inline auto SoundInfo::GetGoomPower() const -> float
{
  return m_goomPower;
}

inline auto SoundInfo::GetGoomLimit() const -> float
{
  return m_goomLimit;
}

inline auto SoundInfo::GetBigGoomLimit() const -> float
{
  return m_bigGoomLimit;
}

} // namespace GOOM
