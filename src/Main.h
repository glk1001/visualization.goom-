#pragma once

/*
 *  Copyright (C) 2005-2022 Team Kodi (https://kodi.tv)
 *  Copyright (C) 2005-2013 Team XBMC
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

//#define SAVE_AUDIO_BUFFERS

#include "CircularBuffer.h"
#include "gl_render_types.h"
#include "goom/goom_config.h"
#include "goom/goom_control.h"
#include "goom/goom_graphic.h"
#include "goom/sound_info.h"
#ifdef SAVE_AUDIO_BUFFERS
#include "src/goom/src/utils/buffer_saver.h"
#endif

#include <array>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <glm/gtc/type_ptr.hpp>
#include <kodi/addon-instance/Visualization.h>
#ifdef TARGET_DARWIN
#define GL_SILENCE_DEPRECATION
#endif
#include <kodi/gui/gl/Shader.h>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

// TODO(glk) Fix this properly!
#if !defined(ATTRIBUTE_HIDDEN)
#define ATTRIBUTE_HIDDEN
#endif

namespace GOOM
{
class GoomLogger;
class GlRenderer;
class KodiShaderWithEffects;
}

class ATTRIBUTE_HIDDEN CVisualizationGoom : public kodi::addon::CAddonBase,
                                            public kodi::addon::CInstanceVisualization,
                                            public kodi::gui::gl::CShaderProgram
{
public:
  CVisualizationGoom();
  CVisualizationGoom(const CVisualizationGoom&) noexcept = delete;
  CVisualizationGoom(CVisualizationGoom&&) noexcept      = delete;
  ~CVisualizationGoom() override;
  auto operator=(const CVisualizationGoom&) -> CVisualizationGoom& = delete;
  auto operator=(CVisualizationGoom&&) -> CVisualizationGoom&      = delete;

  [[nodiscard]] auto Start(int numChannels,
                           int samplesPerSec,
                           int bitsPerSample,
                           const std::string& songName) -> bool override;
  auto Stop() -> void override;
  auto Render() -> void override;
  auto AudioData(const float* audioData, size_t audioDataLength) -> void override;
  [[nodiscard]] auto UpdateTrack(const kodi::addon::VisualizationTrack& track) -> bool override;

protected:
  static constexpr size_t MAX_ACTIVE_QUEUE_LENGTH = 20;
  struct PixelBufferData
  {
    std::shared_ptr<GOOM::PixelBuffer> pixelBuffer;
    GOOM::GoomShaderVariables goomShaderVariables;
  };

  static constexpr size_t NUM_AUDIO_BUFFERS_IN_CIRCULAR_BUFFER = 16;
  [[nodiscard]] auto GetGoomControl() const -> const GOOM::GoomControl& { return *m_goomControl; };
  [[nodiscard]] auto GetGoomControl() -> GOOM::GoomControl& { return *m_goomControl; };
  [[nodiscard]] auto AudioBufferLen() const -> size_t { return m_audioBufferLen; };
  [[nodiscard]] auto GoomBufferLen() const -> size_t { return m_goomBufferLen; };
  [[nodiscard]] auto NumChannels() const -> size_t { return m_numChannels; };
  virtual auto NoActiveBufferAvailable() -> void {}
  virtual auto AudioDataQueueTooBig() -> void {}
  virtual auto SkippedAudioData() -> void {}
  virtual auto AudioDataIncorrectReadLength() -> void {}
  virtual auto UpdateGoomBuffer(const std::vector<float>& floatAudioData,
                                PixelBufferData& pixelBufferData) -> void;

private:
  const GOOM::TextureBufferDimensions m_textureBufferDimensions;
  const size_t m_goomBufferLen;
  const size_t m_textureBufferSize;

  size_t m_numChannels                                        = 0;
  size_t m_audioBufferLen                                     = 0;
  uint32_t m_audioBufferNum                                   = 0;
  static constexpr uint32_t MIN_AUDIO_BUFFERS_BEFORE_STARTING = 6;

  // The Goom object
  const GOOM::GoomControl::ShowTitleType m_showTitle;
  std::unique_ptr<GOOM::GoomControl> m_goomControl{};
  std::unique_ptr<GOOM::GoomLogger> m_goomLogger;

  std::unique_ptr<GOOM::KodiShaderWithEffects> m_glShader;
  std::unique_ptr<GOOM::GlRenderer> m_glRenderer;
  class PixelBufferGetter;
  std::unique_ptr<PixelBufferGetter> m_pixelBufferGetter;
  auto SetNextRenderData() noexcept -> void;
  auto DoRender() noexcept -> void;

  // Audio buffer storage
  static constexpr size_t CIRCULAR_BUFFER_SIZE = NUM_AUDIO_BUFFERS_IN_CIRCULAR_BUFFER *
                                                 GOOM::AudioSamples::NUM_AUDIO_SAMPLES *
                                                 GOOM::AudioSamples::AUDIO_SAMPLE_LEN;
  CircularBuffer<float> m_audioBuffer{CIRCULAR_BUFFER_SIZE};

  // Goom process thread handles
  bool m_workerThreadExit = false;
  std::thread m_workerThread{};
  std::mutex m_mutex{};
  std::condition_variable m_wait{};

  auto SetNumChannels(int numChannels) -> void;
  auto StartLogging() -> void;
  auto InitGoomController() -> void;
  auto DeinitGoomController() -> void;
  auto StartGoomProcessBuffersThread() -> void;
  auto StopGoomProcessBuffersThread() -> void;
  auto ExitWorkerThread() -> void;
  auto Process() -> void;
  [[nodiscard]] auto GetNextActivePixelBufferData() -> PixelBufferData;
  auto PushUsedPixels(const PixelBufferData& pixelBufferData) -> void;

  [[nodiscard]] auto MakePixelBufferData() const -> PixelBufferData;
  // Screen frames storage: active queue for next view and stored queue to
  // use for the next goom update.
  std::queue<PixelBufferData> m_activePixelBufferDataQueue{};
  std::queue<PixelBufferData> m_storedPixelBufferDataQueue{};
  auto StartActivePixelBufferDataQueue() -> void;

  // Start flag to know init was OK
  bool m_started = false;

  [[nodiscard]] auto StartWithCatch(int numChannels,
                                    int samplesPerSec,
                                    int bitsPerSample,
                                    const std::string& songName) -> bool;
  [[nodiscard]] auto StartWithNoCatch(int numChannels,
                                      int samplesPerSec,
                                      int bitsPerSample,
                                      const std::string& songName) -> bool;
  [[nodiscard]] auto StartVis(int numChannels,
                              int samplesPerSec,
                              int bitsPerSample,
                              const std::string& songName) -> bool;

  auto StopWithCatch() -> void;
  auto StopWithNoCatch() -> void;
  auto StopVis() -> void;

  auto ProcessWithCatch() -> void;
  auto ProcessWithNoCatch() -> void;
  auto ProcessVis() -> void;

#ifdef SAVE_AUDIO_BUFFERS
  using AudioBufferWriter = GOOM::UTILS::BufferSaver<float>;
  [[nodiscard]] static auto GetAudioBufferWriter(const std::string& songName)
      -> std::unique_ptr<AudioBufferWriter>;
  std::unique_ptr<AudioBufferWriter> m_audioBufferWriter{};
  auto SaveAudioBuffer(const std::vector<float>& floatAudioData) -> void;
#endif

  static auto HandleError(const std::string& errorMsg) -> void;
};
