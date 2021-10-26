#pragma once

/*
 *  Copyright (C) 2005-2022 Team Kodi (https://kodi.tv)
 *  Copyright (C) 2005-2013 Team XBMC
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "CircularBuffer.h"
#include "goom/goom_config.h"
#include "goom/goom_control.h"
#include "goom/goom_graphic.h"
#include "goom/sound_info.h"

#include <array>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <glm/gtc/type_ptr.hpp>
#include <kodi/addon-instance/Visualization.h>
#include <kodi/gui/gl/Shader.h>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

class ATTRIBUTE_HIDDEN CVisualizationGoom : public kodi::addon::CAddonBase,
                                            public kodi::addon::CInstanceVisualization,
                                            public kodi::gui::gl::CShaderProgram
{
public:
  CVisualizationGoom();
  CVisualizationGoom(const CVisualizationGoom&) noexcept = delete;
  CVisualizationGoom(CVisualizationGoom&&) noexcept = delete;
  ~CVisualizationGoom() override;
  auto operator=(const CVisualizationGoom&) -> CVisualizationGoom& = delete;
  auto operator=(CVisualizationGoom&&) -> CVisualizationGoom& = delete;

  [[nodiscard]] auto Start(int numChannels,
                           int samplesPerSec,
                           int bitsPerSample,
                           const std::string& songName) -> bool override;
  void Stop() override;
  [[nodiscard]] auto IsDirty() -> bool override;
  void Render() override;
  void AudioData(const float* audioData, size_t audioDataLength) override;
  [[nodiscard]] auto UpdateTrack(const kodi::addon::VisualizationTrack& track) -> bool override;

  // kodi::gui::gl::CShaderProgram
  void OnCompiledAndLinked() override;
  [[nodiscard]] auto OnEnabled() -> bool override;

protected:
  static constexpr size_t NUM_AUDIO_BUFFERS_IN_CIRCULAR_BUFFER = 16;
  [[nodiscard]] auto GetGoomControl() const -> const GOOM::GoomControl& { return *m_goomControl; };
  [[nodiscard]] auto GetGoomControl() -> GOOM::GoomControl& { return *m_goomControl; };
  [[nodiscard]] auto AudioBufferLen() const -> size_t { return m_audioBufferLen; };
  [[nodiscard]] auto TexWidth() const -> uint32_t { return m_textureWidth; };
  [[nodiscard]] auto TexHeight() const -> uint32_t { return m_textureHeight; };
  [[nodiscard]] auto GoomBufferLen() const -> size_t { return m_goomBufferLen; };
  [[nodiscard]] auto CurrentSongName() const -> const std::string& { return m_currentSongName; };
  [[nodiscard]] auto NumChannels() const -> size_t { return m_numChannels; };
  virtual void NoActiveBufferAvailable() {}
  virtual void AudioDataQueueTooBig() {}
  virtual void SkippedAudioData() {}
  virtual void AudioDataIncorrectReadLength() {}
  virtual void UpdateGoomBuffer(const std::string& title,
                                const std::vector<float>& floatAudioData,
                                std::shared_ptr<GOOM::PixelBuffer>& pixels);

private:
  const int m_windowWidth;
  const int m_windowHeight;
  const int m_windowXPos;
  const int m_windowYPos;

  const uint32_t m_textureWidth;
  const uint32_t m_textureHeight;
  const size_t m_goomBufferLen;
  const size_t m_goomBufferSize;

  size_t m_numChannels{};
  size_t m_audioBufferLen = 0;
  uint32_t m_audioBufferNum = 0;
  static constexpr uint32_t MIN_AUDIO_BUFFERS_BEFORE_STARTING = 6;

  std::string m_currentSongName{};
  std::string m_lastSongName{};
  const GOOM::GoomControl::ShowTitleType m_showTitle;
  bool m_titleChange = false;
  [[nodiscard]] auto GetTitle() -> std::string;

  GLint m_componentsPerVertex = 2;
  GLint m_componentsPerTexel = 2;
  int m_numVertices = 2 * 3; // 2 triangles
  const std::vector<GLfloat> m_quadData;
  [[nodiscard]] static auto GetGlQuadData(int width, int height, int xPos, int yPos)
      -> std::vector<GLfloat>;

  const bool m_usePixelBufferObjects;
  // Note: 'true' is supposed to give better performance, but it's not obvious.
  // And when 'true', there may be issues with screen refreshes when changing windows in Kodi.

  GLuint m_textureId = 0;
  static const int G_NUM_PBOS = 3;
  std::array<GLuint, G_NUM_PBOS> m_pboIds{};
  std::array<uint8_t*, G_NUM_PBOS> m_pboGoomBuffer{};
  size_t m_currentPboIndex = 0;
  glm::mat4 m_projModelMatrix{};
  GLuint m_vaoObject = 0;
  GLuint m_vertexVBO = 0;
  GLint m_uProjModelMatLoc = -1;
  GLint m_aPositionLoc = -1;
  GLint m_aCoordLoc = -1;
  GLint m_uTexExposureLoc = -1;

  // Goom's data itself
  std::unique_ptr<GOOM::GoomControl> m_goomControl{};

  // Audio buffer storage
  //static constexpr size_t CIRCULAR_BUFFER_SIZE = NUM_AUDIO_BUFFERS_IN_CIRCULAR_BUFFER *
  //                                               GOOM::AudioSamples::NUM_AUDIO_SAMPLES *
  //                                               GOOM::AudioSamples::AUDIO_SAMPLE_LEN;
  //CircularBuffer<float> m_buffer(CIRCULAR_BUFFER_SIZE);
  static const size_t CIRCULAR_BUFFER_SIZE;
  CircularBuffer<float> m_audioBuffer{static_cast<uint32_t>(CIRCULAR_BUFFER_SIZE)};

  // Goom process thread handles
  bool m_workerThreadExit = false;
  std::thread m_workerThread{};
  std::mutex m_mutex{};
  std::condition_variable m_wait{};

  // Screen frames storage, m_activeQueue for next view and m_storedQueue to
  // use on next goom round become active again.
protected:
  static constexpr size_t MAX_ACTIVE_QUEUE_LENGTH = 20;

private:
  void SetNumChannels(int numChannels);
  void SetSongTitle(const std::string& songTitle);
  static void StartLogging() ;
  [[nodiscard]] auto InitGoomController() -> bool;
  void DeinitGoomController();
  void StartGoomProcessBuffersThread();
  void StopGoomProcessBuffersThread();
  void Process();
  [[nodiscard]] auto GetNextActivePixels() -> std::shared_ptr<GOOM::PixelBuffer>;
  void PushUsedPixels(const std::shared_ptr<GOOM::PixelBuffer>& pixels);

  [[nodiscard]] auto InitGl() -> bool;
  void DeinitGl();
  [[nodiscard]] auto InitGlShaders() -> bool;
  [[nodiscard]] auto InitGlObjects() -> bool;
  void SetupGlVertexAttributes();
  [[nodiscard]] auto CreateGlTexture() -> bool;
  [[nodiscard]] auto SetupGlPixelBufferObjects() -> bool;
  void RenderGlPixelBuffer(const GOOM::PixelBuffer& pixelBuffer);

  std::queue<std::shared_ptr<GOOM::PixelBuffer>> m_activeQueue{};
  std::queue<std::shared_ptr<GOOM::PixelBuffer>> m_storedQueue{};
  void StartActiveQueue();

  // Start flag to know init was OK
  bool m_started = false;
};
