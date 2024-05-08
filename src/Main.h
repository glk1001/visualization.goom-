#pragma once

/*
 *  Copyright (C) 2005-2023 Team Kodi (https://kodi.tv)
 *  Copyright (C) 2005-2013 Team XBMC
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "goom/circular_buffer.h"
#include "goom/goom_config.h" // NOLINT: Always want this
#include "goom/sound_info.h"

#ifdef TARGET_DARWIN
#define GL_SILENCE_DEPRECATION
#endif

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4242) // kodi/tools/StringUtils.h: possible loss of data
#pragma warning(disable : 4702) // kodi/gui/gl/Shader.h: unreachable code
#endif

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <cstddef>
#include <cstdint>
#include <kodi/AddonBase.h>
#include <kodi/addon-instance/Visualization.h>
#include <kodi/gui/gl/Shader.h>
#include <memory>
#include <span>
#include <string>
#include <vector>

#if !defined(ATTRIBUTE_HIDDEN)
#define ATTRIBUTE_HIDDEN
#endif

namespace GOOM
{
class GoomLogger;
}

import Goom.GoomVisualization;

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

  auto AudioData(const float* audioData, size_t audioDataLength) -> void override;
  [[nodiscard]] auto UpdateTrack(const kodi::addon::VisualizationTrack& track) -> bool override;

  auto Render() -> void override;

private:
  bool m_started = false;
  std::unique_ptr<GOOM::GoomLogger> m_goomLogger;
  static constexpr auto CONSUME_WAIT_FOR_PRODUCER_MS = 20U;
  GOOM::VIS::GoomVisualization m_goomVisualization;

  auto StartLogging() -> void;

  auto StartWithCatch(int numChannels) -> void;
  auto StartWithoutCatch(int numChannels) -> void;
  auto StartVis(int numChannels) -> void;

  auto StopWithCatch() -> void;
  auto StopWithoutCatch() -> void;
  auto StopVis() -> void;

  auto PassSettings() noexcept -> void;

  size_t m_audioSampleLen     = 0U;
  uint32_t m_dropAudioDataNum = 0U;
  std::vector<float> m_rawAudioData{};
  static constexpr auto NUM_AUDIO_BUFFERS_IN_CIRCULAR_BUFFER = 128U;
  static constexpr size_t CIRCULAR_BUFFER_SIZE = NUM_AUDIO_BUFFERS_IN_CIRCULAR_BUFFER *
                                                 GOOM::AudioSamples::NUM_AUDIO_SAMPLES *
                                                 GOOM::AudioSamples::AUDIO_SAMPLE_LEN;
  GOOM::CircularBuffer<float> m_audioBuffer{CIRCULAR_BUFFER_SIZE};
  auto InitAudioData(int32_t numChannels) noexcept -> void;
  auto AddAudioDataToBuffer(std::span<const float> audioData) noexcept -> void;
  auto MoveNextAudioSampleToProducer() noexcept -> void;

  [[nodiscard]] auto ReadyToRender() const -> bool;
  auto DoRender() noexcept -> void;
  double m_totalRenderTimeInMs = 0.0;
  uint64_t m_numRenders        = 0U;

#ifdef GOOM_DEBUG
  [[noreturn]]
#endif
  auto
  HandleError(const std::string& errorMsg) -> void;
};
