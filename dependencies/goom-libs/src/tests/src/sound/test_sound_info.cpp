#include "catch2/catch.hpp"
#include "sound_info.h"
#include "utils/mathutils.h"

#include <algorithm>
#include <memory>

using GOOM::AudioSamples;
using GOOM::SoundInfo;
using GOOM::UTILS::floats_equal;

[[nodiscard]] static auto GetAudioData(const float xMin0,
                                       const float xMax0,
                                       const float xMin1,
                                       const float xMax1) -> std::vector<float>
{
  std::vector<float> audioData(AudioSamples::NUM_AUDIO_SAMPLES * AudioSamples::AUDIO_SAMPLE_LEN);

  float x = xMin0;
  float xStep = (xMax0 - xMin0) / static_cast<float>(AudioSamples::AUDIO_SAMPLE_LEN - 1);
  for (size_t i = 0; i < audioData.size(); i += 2)
  {
    audioData.at(i) = x;
    x += xStep;
  }

  x = xMin1;
  xStep = (xMax1 - xMin1) / static_cast<float>(AudioSamples::AUDIO_SAMPLE_LEN - 1);
  for (size_t i = 1; i < audioData.size(); i += 2)
  {
    audioData.at(i) = x;
    x += xStep;
  }

  return audioData;
}


//constexpr size_t NUM_SAMPLE_CHANNELS = AudioSamples::NUM_AUDIO_SAMPLES;
const size_t NUM_SAMPLE_CHANNELS = AudioSamples::NUM_AUDIO_SAMPLES;
constexpr float X_MIN0 = -0.9F;
constexpr float X_MAX0 = +0.1F;
constexpr float X_MIN1 = +0.2F;
constexpr float X_MAX1 = +0.9F;

constexpr auto EXPECTED_X_MIN0 = X_MIN0;
constexpr auto EXPECTED_X_MAX0 = X_MAX0;
constexpr auto EXPECTED_X_MIN1 = X_MIN1;
constexpr auto EXPECTED_X_MAX1 = X_MAX1;

TEST_CASE("Test AudioSamples MinMax", "[AudioSamplesMinMax]")
{
  const std::vector<float> audioData = GetAudioData(X_MIN0, X_MAX0, X_MIN1, X_MAX1);
  const AudioSamples audioSamples{NUM_SAMPLE_CHANNELS, audioData};

  UNSCOPED_INFO("NUM_SAMPLE_CHANNELS = " << NUM_SAMPLE_CHANNELS);

  REQUIRE(AudioSamples::NUM_AUDIO_SAMPLES == 2);
  REQUIRE(AudioSamples::AUDIO_SAMPLE_LEN == 512);
  REQUIRE(audioSamples.GetNumDistinctChannels() == NUM_SAMPLE_CHANNELS);

  UNSCOPED_INFO("X_MIN0 = " << X_MIN0);
  UNSCOPED_INFO("EXPECTED_X_MIN0 = " << EXPECTED_X_MIN0);
  UNSCOPED_INFO("get_minimum0 = " << *std::min_element(begin(audioSamples.GetSample(0)), end(audioSamples.GetSample(0))));
  UNSCOPED_INFO("expected get_minimum = " << *std::min_element(begin(audioData), end(audioData)));
  REQUIRE(floats_equal(audioSamples.GetSampleMinMax(0).minVal, AudioSamples::GetPositiveValue(EXPECTED_X_MIN0)));

  UNSCOPED_INFO("X_MAX0 = " << X_MAX0);
  UNSCOPED_INFO("EXPECTED_X_MAX0 = " << EXPECTED_X_MAX0);
  UNSCOPED_INFO("get_maximum0 = " << *std::max_element(begin(audioSamples.GetSample(0)), end(audioSamples.GetSample(0))));
  UNSCOPED_INFO("expected get_maximum = " << *std::max_element(begin(audioData), end(audioData)));
  REQUIRE(floats_equal(audioSamples.GetSampleMinMax(0).maxVal, AudioSamples::GetPositiveValue(EXPECTED_X_MAX0)));

  UNSCOPED_INFO("X_MIN1 = " << X_MIN1);
  UNSCOPED_INFO("EXPECTED_X_MIN1 = " << EXPECTED_X_MIN1);
  UNSCOPED_INFO("get_minimum1 = " << *std::min_element(begin(audioSamples.GetSample(1)), end(audioSamples.GetSample(1))));
  UNSCOPED_INFO("expected get_minimum = " << *std::min_element(begin(audioData), end(audioData)));
  REQUIRE(floats_equal(audioSamples.GetSampleMinMax(1).minVal, AudioSamples::GetPositiveValue(EXPECTED_X_MIN1)));

  UNSCOPED_INFO("X_MAX1 = " << X_MAX1);
  UNSCOPED_INFO("EXPECTED_X_MAX1 = " << EXPECTED_X_MAX1);
  UNSCOPED_INFO("get_maximum1 = " << *std::max_element(begin(audioSamples.GetSample(1)), end(audioSamples.GetSample(1))));
  UNSCOPED_INFO("expected get_maximum = " << *std::max_element(begin(audioData), end(audioData)));
  REQUIRE(floats_equal(audioSamples.GetSampleMinMax(1).maxVal, AudioSamples::GetPositiveValue(EXPECTED_X_MAX1)));
}

TEST_CASE("Test AudioSamples Arrays", "[AudioSamplesArrays]")
{
  const std::vector<float> audioData = GetAudioData(X_MIN0, X_MAX0, X_MIN1, X_MAX1);
  const AudioSamples audioSamples{NUM_SAMPLE_CHANNELS, audioData};

  UNSCOPED_INFO("X_MIN0 = " << X_MIN0);
  REQUIRE(floats_equal(audioData.at(0), X_MIN0));

  UNSCOPED_INFO("EXPECTED_X_MIN0 = " << EXPECTED_X_MIN0);
  REQUIRE(floats_equal(audioSamples.GetSample(0).at(0), AudioSamples::GetPositiveValue(EXPECTED_X_MIN0)));

  UNSCOPED_INFO("X_MAX0 = " << X_MAX0);
  REQUIRE(floats_equal(audioData.at(audioData.size() - 2), X_MAX0));

  UNSCOPED_INFO("EXPECTED_X_MAX0 = " << EXPECTED_X_MAX0);
  REQUIRE(floats_equal(audioSamples.GetSample(0).at(AudioSamples::AUDIO_SAMPLE_LEN - 1), AudioSamples::GetPositiveValue(EXPECTED_X_MAX0)));

  UNSCOPED_INFO("X_MIN1 = " << X_MIN1);
  REQUIRE(floats_equal(audioData.at(1), X_MIN1));

  UNSCOPED_INFO("EXPECTED_X_MIN1 = " << EXPECTED_X_MIN1);
  REQUIRE(floats_equal(audioSamples.GetSample(1).at(0), AudioSamples::GetPositiveValue(EXPECTED_X_MIN1)));

  UNSCOPED_INFO("X_MAX1 = " << X_MAX1);
  REQUIRE(floats_equal(audioData.at(audioData.size() - 1), X_MAX1));

  UNSCOPED_INFO("EXPECTED_X_MAX1 = " << EXPECTED_X_MAX1);
  REQUIRE(floats_equal(audioSamples.GetSample(1).at(AudioSamples::AUDIO_SAMPLE_LEN - 1), AudioSamples::GetPositiveValue(EXPECTED_X_MAX1)));
}

TEST_CASE("Test SoundInfo ProcessSample Defaults", "[SoundInfoProcessSampleDefaults]")
{
  SoundInfo soundInfo{};

  const std::vector<float> audioData = GetAudioData(X_MIN0, X_MAX0, X_MIN1, X_MAX1);
  const AudioSamples audioSamples{NUM_SAMPLE_CHANNELS, audioData};

  soundInfo.ProcessSample(audioSamples);

  REQUIRE(floats_equal(soundInfo.GetVolume(), AudioSamples::GetPositiveValue(X_MAX1)));
  REQUIRE(soundInfo.GetTimeSinceLastGoom() == 1);
  REQUIRE(soundInfo.GetTimeSinceLastBigGoom() == 1);
  REQUIRE(soundInfo.GetTotalGoomsInCurrentCycle() == 0);
}

TEST_CASE("Test SoundInfo Volume", "[SoundInfoVolume]")
{
  SoundInfo soundInfo{};

  std::vector<float> audioData = GetAudioData(X_MIN0, X_MAX0, X_MIN1, X_MAX1);
  auto audioSamples = std::make_unique<AudioSamples>(NUM_SAMPLE_CHANNELS, audioData);

  // First update - defaults
  constexpr float ALL_TIMES_MAX = AudioSamples::GetPositiveValue(EXPECTED_X_MAX1);
  constexpr float ALL_TIMES_MIN = AudioSamples::GetPositiveValue(EXPECTED_X_MIN0);
  soundInfo.ProcessSample(*audioSamples);
  UNSCOPED_INFO("ALL_TIMES_MAX = " << ALL_TIMES_MAX);
  UNSCOPED_INFO("soundInfo.GetVolume() = " << soundInfo.GetVolume());
  REQUIRE(floats_equal(soundInfo.GetVolume(), ALL_TIMES_MAX));
  REQUIRE(floats_equal(soundInfo.GetAllTimesMaxVolume(), ALL_TIMES_MAX));
  REQUIRE(floats_equal(soundInfo.GetAllTimesMinVolume(), ALL_TIMES_MIN));

  // Second update - flat line, new volume
  constexpr float FLAT_VOL = 0.1F;
  constexpr float EXPECTED_NEW_VOLUME = AudioSamples::GetPositiveValue(FLAT_VOL);
  UNSCOPED_INFO("EXPECTED_NEW_VOLUME = " << EXPECTED_NEW_VOLUME);

  std::fill(begin(audioData), end(audioData), FLAT_VOL);
  audioSamples = std::make_unique<AudioSamples>(NUM_SAMPLE_CHANNELS, audioData);
  soundInfo.ProcessSample(*audioSamples);
  UNSCOPED_INFO("soundInfo.GetVolume() = " << soundInfo.GetVolume());
  REQUIRE(floats_equal(soundInfo.GetVolume(), EXPECTED_NEW_VOLUME));
  REQUIRE(floats_equal(soundInfo.GetAllTimesMaxVolume(), ALL_TIMES_MAX));
  REQUIRE(floats_equal(soundInfo.GetAllTimesMinVolume(), ALL_TIMES_MIN));

  // Third update - new max volume
  constexpr float NEW_MAX_VOL = X_MAX1 + 0.02F;
  audioData.at(10) = NEW_MAX_VOL;
  audioSamples = std::make_unique<AudioSamples>(NUM_SAMPLE_CHANNELS, audioData);
  soundInfo.ProcessSample(*audioSamples);
  REQUIRE(floats_equal(soundInfo.GetVolume(), AudioSamples::GetPositiveValue(NEW_MAX_VOL)));
  REQUIRE(floats_equal(soundInfo.GetAllTimesMaxVolume(), AudioSamples::GetPositiveValue(NEW_MAX_VOL)));
  REQUIRE(floats_equal(soundInfo.GetAllTimesMinVolume(), ALL_TIMES_MIN));

  // Fourth update - negative sound values
  constexpr float NEGATIVE_VOL = -0.2F;
  std::fill(begin(audioData), end(audioData), NEGATIVE_VOL);
  audioSamples = std::make_unique<AudioSamples>(NUM_SAMPLE_CHANNELS, audioData);
  soundInfo.ProcessSample(*audioSamples);
  REQUIRE(floats_equal(soundInfo.GetVolume(), AudioSamples::GetPositiveValue(NEGATIVE_VOL)));
  REQUIRE(floats_equal(soundInfo.GetAllTimesMaxVolume(), AudioSamples::GetPositiveValue(NEW_MAX_VOL)));
  REQUIRE(floats_equal(soundInfo.GetAllTimesMinVolume(), ALL_TIMES_MIN));
}
