#pragma once

// #define SAVE_AUDIO_BUFFERS

#include "CircularBuffer.h"
#include "gl_render_types.h"
#include "goom/goom_control.h"
#include "goom/goom_graphic.h"
#include "goom/sound_info.h"

#ifdef SAVE_AUDIO_BUFFERS
#include "src/goom/src/utils/buffer_saver.h"
#endif

#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

namespace GOOM
{

class GoomLogger;

class GoomBufferProducer
{
public:
  struct PixelBufferData
  {
    std::shared_ptr<GOOM::PixelBuffer> pixelBuffer;
    GOOM::GoomShaderVariables goomShaderVariables;
  };

  GoomBufferProducer(const TextureBufferDimensions& textureBufferDimensions,
                     const std::string& resourcesDirectory,
                     GoomControl::ShowTitleType showTitle,
#ifdef SAVE_AUDIO_BUFFERS
                     const std::string& audioBuffersSaveDir,
#endif
                     GoomLogger& goomLogger) noexcept;

  auto SetShowGoomState(bool value) -> void;
  auto SetDumpDirectory(const std::string& dumpDirectory) -> void;
  auto SetNumChannels(uint32_t value) -> void;

  auto Start() -> void;
  auto Stop() -> void;

  [[nodiscard]] auto GetAudioBufferNum() const noexcept -> uint32_t;

  auto ProcessAudioData(const float* audioData, size_t audioDataLength) -> void;
  auto UpdateTrack(const GoomControl::SongInfo& track) -> void;

  [[nodiscard]] auto GetNextActivePixelBufferData() -> PixelBufferData;
  auto PushUsedPixels(const PixelBufferData& pixelBufferData) -> void;

private:
  TextureBufferDimensions m_textureBufferDimensions;
  GoomLogger* m_goomLogger;
  bool m_started = false;

  // Audio buffer storage
  size_t m_numChannels                                       = 0;
  size_t m_audioBufferLen                                    = 0;
  uint32_t m_audioBufferNum                                  = 0;
  static constexpr auto NUM_AUDIO_BUFFERS_IN_CIRCULAR_BUFFER = 16U;
  static constexpr size_t CIRCULAR_BUFFER_SIZE = NUM_AUDIO_BUFFERS_IN_CIRCULAR_BUFFER *
                                                 AudioSamples::NUM_AUDIO_SAMPLES *
                                                 AudioSamples::AUDIO_SAMPLE_LEN;
  CircularBuffer<float> m_audioBuffer{CIRCULAR_BUFFER_SIZE};

  // Goom buffer storage
  static constexpr size_t MAX_ACTIVE_QUEUE_LENGTH = 20;
  // Screen frames storage: active queue for next view and stored queue to
  // use for the next goom update.
  std::queue<PixelBufferData> m_activePixelBufferDataQueue{};
  std::queue<PixelBufferData> m_storedPixelBufferDataQueue{};
  auto StartActivePixelBufferDataQueue() -> void;
  [[nodiscard]] auto MakePixelBufferData() const -> PixelBufferData;

  std::thread m_processBuffersThread{};
  std::mutex m_mutex{};
  std::condition_variable m_wait{};
  bool m_processGoomBuffersThreadFinished = false;

  auto StartProcessGoomBuffersThread() -> void;
  auto StopProcessGoomBuffersThread() -> void;
  auto ProcessGoomBuffersThread() -> void;

  //TODO(glk) Can split goom control out and make it a strategy.
  std::unique_ptr<GOOM::GoomControl> m_goomControl;
  auto UpdateGoomBuffer(const std::vector<float>& floatAudioData, PixelBufferData& pixelBufferData)
      -> void;

#ifdef SAVE_AUDIO_BUFFERS
  const std::string m_audioBuffersSaveDir;
  using AudioBufferWriter = GOOM::UTILS::BufferSaver<float>;
  [[nodiscard]] auto GetAudioBufferWriter(const std::string& songName) const
      -> std::unique_ptr<AudioBufferWriter>;
  std::unique_ptr<AudioBufferWriter> m_audioBufferWriter{};
  auto SaveAudioBuffer(const std::vector<float>& floatAudioData) -> void;
#endif
};

inline auto GoomBufferProducer::SetShowGoomState(const bool value) -> void
{
  m_goomControl->SetShowGoomState(value);
}

inline auto GoomBufferProducer::SetDumpDirectory(const std::string& dumpDirectory) -> void
{
  m_goomControl->SetDumpDirectory(dumpDirectory);
}

inline auto GoomBufferProducer::GetAudioBufferNum() const noexcept -> uint32_t
{
  return m_audioBufferNum;
}

} // namespace GOOM
