#pragma once

/*
 *  Copyright (C) 2005-2023 Team Kodi (https://kodi.tv)
 *  Copyright (C) 2005-2013 Team XBMC
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "gl_renderer.h"
#include "goom/goom_config.h"
#include "goom_buffer_producer.h"

#ifdef TARGET_DARWIN
#define GL_SILENCE_DEPRECATION
#endif

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4242) // kodi/tools/StringUtils.h: possible loss of data
#pragma warning(disable : 4702) // kodi/gui/gl/Shader.h: unreachable code
#endif

#include <kodi/addon-instance/Visualization.h>
#include <kodi/gui/gl/Shader.h>

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <memory>
#include <string>

// TODO(glk) Fix this properly!
#if !defined(ATTRIBUTE_HIDDEN)
#define ATTRIBUTE_HIDDEN
#endif

namespace GOOM
{
class GoomLogger;
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

  auto AudioData(const float* audioData, size_t audioDataLength) -> void override;
  [[nodiscard]] auto UpdateTrack(const kodi::addon::VisualizationTrack& track) -> bool override;

  auto Render() -> void override;

private:
  bool m_started = false;
  std::unique_ptr<GOOM::GoomLogger> m_goomLogger;

  auto StartLogging() -> void;

  auto StartWithCatch(int numChannels) -> void;
  auto StartWithoutCatch(int numChannels) -> void;
  auto StartVis(int numChannels) -> void;

  auto StopWithCatch() -> void;
  auto StopWithoutCatch() -> void;
  auto StopVis() -> void;

  GOOM::GlRenderer m_glRenderer;
  GOOM::GoomBufferProducer m_goomBufferProducer;
  class PixelBufferGetter;
  std::unique_ptr<PixelBufferGetter> m_pixelBufferGetter;
  auto DoRender() noexcept -> void;

#ifdef GOOM_DEBUG
  [[noreturn]]
#endif
  auto
  HandleError(const std::string& errorMsg) -> void;
};
