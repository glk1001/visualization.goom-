/*
 *  Copyright (C) 2005-2022 Team Kodi (https://kodi.tv)
 *  Copyright (C) 2005-2013 Team XBMC
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#undef NO_LOGGING

#include "Main.h"

#include "gl_render_types.h"
#include "goom/goom_config.h"
#include "goom/goom_control.h"
#include "goom/goom_logger.h"
#include "goom/goom_types.h"
#include "goom_visualization.h"

#include <format>
#include <kodi/Filesystem.h>
#include <random>
#include <span>
#include <stdexcept>
#include <string>

using GOOM::AudioSamples;
using GOOM::GoomControl;
using GOOM::GoomLogger;
using GOOM::ShowSongTitleType;
using GOOM::TextureBufferDimensions;
using GOOM::WindowDimensions;
using GOOM::VIS::GoomVisualization;

#ifndef IS_KODI_BUILD
static_assert(false, "This is a Kodi file: 'IS_KODI_BUILD' should be defined.");
#endif

namespace KODI_ADDON = kodi::addon;
using AddonLogEnum   = ADDON_LOG;

#define DEBUG_LOGGING 0
//#define DEBUG_LOGGING 1

namespace
{

constexpr auto MAX_QUALITY = 4;
constexpr std::array WIDTHS_BY_QUALITY{
    512U,
    640U,
    1280U,
    1600U,
    1920U,
};
static_assert(WIDTHS_BY_QUALITY.size() == (MAX_QUALITY + 1));

constexpr std::array HEIGHTS_BY_QUALITY{
    256U,
    360U,
    720U,
    900U,
    1080U,
};
static_assert(HEIGHTS_BY_QUALITY.size() == WIDTHS_BY_QUALITY.size());

constexpr auto* GOOM_ADDON_DATA_DIR = "special://userdata/addon_data/visualization.goom-pp";

constexpr auto* QUALITY_SETTING         = "quality";
constexpr auto* SHOW_TITLE_SETTING      = "show_title";
constexpr auto* SHOW_GOOM_STATE_SETTING = "show_goom_state";
constexpr auto* GOOM_DUMPS_SETTING      = "goom_dumps";

[[nodiscard]] inline auto GetTextureBufferDimensions() noexcept -> TextureBufferDimensions
{
  return {WIDTHS_BY_QUALITY.at(static_cast<size_t>(
              std::min(MAX_QUALITY, KODI_ADDON::GetSettingInt(QUALITY_SETTING)))),
          HEIGHTS_BY_QUALITY.at(static_cast<size_t>(
              std::min(MAX_QUALITY, KODI_ADDON::GetSettingInt(QUALITY_SETTING))))};
}
} // namespace

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, hicpp-vararg)

CVisualizationGoom::CVisualizationGoom()
  : m_goomLogger{GoomControl::MakeGoomLogger()},
    m_goomVisualization{
        *m_goomLogger,
        KODI_ADDON::GetAddonPath(RESOURCES_DIR),
        KODI_ADDON::GetAddonPath(std::string{SHADERS_DIR} + PATH_SEP + GL_TYPE_STRING),
        GetTextureBufferDimensions()}
{
  StartLogging();
  kodi::Log(ADDON_LOG_DEBUG, "CVisualizationGoom: Created CVisualizationGoom object.");

  m_goomVisualization.SetWindowDimensions({Width(), Height()});
}

// NOLINTBEGIN(clang-analyzer-optin.cplusplus.VirtualCall)
CVisualizationGoom::~CVisualizationGoom()
{
  kodi::Log(ADDON_LOG_DEBUG, "CVisualizationGoom: Destroyed CVisualizationGoom object.");
  LogStop(*m_goomLogger);
}
// NOLINTEND(clang-analyzer-optin.cplusplus.VirtualCall)

auto CVisualizationGoom::HandleError(const std::string& errorMsg) -> void
{
  const auto fullMsg = std_fmt::format("CVisualizationGoom: {}", errorMsg);

  LogError(*m_goomLogger, fullMsg);

#ifdef GOOM_DEBUG
  throw std::runtime_error(fullMsg);
#endif
}

auto CVisualizationGoom::PassSettings() noexcept -> void
{
  GoomVisualization::SetRandomSeed(std::random_device{}());

  m_goomVisualization.SetShowGoomState(KODI_ADDON::GetSettingBoolean(SHOW_GOOM_STATE_SETTING));
  m_goomVisualization.SetDumpDirectory(kodi::vfs::TranslateSpecialProtocol(
      std::string(GOOM_ADDON_DATA_DIR) + PATH_SEP + GOOM_DUMPS_SETTING));
  m_goomVisualization.SetShowSongTitle(
      static_cast<ShowSongTitleType>(KODI_ADDON::GetSettingInt(SHOW_TITLE_SETTING)));
}

//-- Start --------------------------------------------------------------------
// Called when a new soundtrack is played
//-----------------------------------------------------------------------------
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
auto CVisualizationGoom::Start(const int numChannels,
                               [[maybe_unused]] const int samplesPerSec,
                               [[maybe_unused]] const int bitsPerSample,
                               [[maybe_unused]] const std::string& songName) -> bool
{
  if (m_started)
  {
    kodi::Log(ADDON_LOG_WARNING,
              "CVisualizationGoom: Already started without a stop - skipping this.");
    return true;
  }

#ifdef GOOM_DEBUG
  StartWithoutCatch(numChannels);
#else
  StartWithCatch(numChannels);
#endif
  return true;
}

inline auto CVisualizationGoom::StartWithoutCatch(const int numChannels) -> void
{
  StartVis(numChannels);
}

auto CVisualizationGoom::StartWithCatch(const int numChannels) -> void
{
  try
  {
    StartVis(numChannels);
  }
  catch (const std::exception& e)
  {
    HandleError(std_fmt::format("CVisualizationGoom start failed: {}", e.what()));
  }
}

//-- Stop ---------------------------------------------------------------------
// Called when the visualisation is closed by Kodi
//-----------------------------------------------------------------------------
auto CVisualizationGoom::Stop() -> void
{
  if (not m_started)
  {
    LogWarn(*m_goomLogger, "CVisualizationGoom: Not started - skipping this.");
    return;
  }

#ifdef GOOM_DEBUG
  StopWithoutCatch();
#else
  StopWithCatch();
#endif
}

inline auto CVisualizationGoom::StopWithoutCatch() -> void
{
  StopVis();
}

auto CVisualizationGoom::StopWithCatch() -> void
{
  try
  {
    StopVis();
  }
  catch (const std::exception& e)
  {
    HandleError(std_fmt::format("Goom stop failed: {}", e.what()));
  }
}

auto CVisualizationGoom::StartVis(const int numChannels) -> void
{
  PassSettings();

  InitAudioData(numChannels);

  m_goomVisualization.Start(numChannels);
  m_goomVisualization.StartThread();

  m_started = true;
}

inline auto CVisualizationGoom::StopVis() -> void
{
  m_started = false;

  m_goomVisualization.Stop();

  LogStop(*m_goomLogger);
}

auto CVisualizationGoom::InitAudioData(const int32_t numChannels) noexcept -> void
{
  m_dropAudioDataNum = 0U;
  m_audioSampleLen   = static_cast<size_t>(numChannels) * AudioSamples::AUDIO_SAMPLE_LEN;
  m_rawAudioData.resize(m_audioSampleLen);
  m_audioBuffer.Clear();
}

auto CVisualizationGoom::StartLogging() -> void
{
  static const auto s_KODI_LOGGER = [](const GoomLogger::LogLevel lvl, const std::string& msg)
  {
    const auto kodiLvl = static_cast<AddonLogEnum>(static_cast<size_t>(lvl));
    kodi::Log(kodiLvl, msg.c_str());
  };
  AddLogHandler(*m_goomLogger, "kodi-logger", s_KODI_LOGGER);
  SetShowDateTime(*m_goomLogger, false);
  LogStart(*m_goomLogger);
}

auto CVisualizationGoom::UpdateTrack(const kodi::addon::VisualizationTrack& track) -> bool
{
  m_goomVisualization.UpdateTrack({track.GetTitle(),
                                   track.GetArtist(),
                                   track.GetAlbumArtist(),
                                   track.GetGenre(),
                                   static_cast<uint32_t>(track.GetDuration())});

  return true;
}

//-- AudioData ----------------------------------------------------------------
// Called by Kodi to pass new audio data to the vis
//-----------------------------------------------------------------------------
auto CVisualizationGoom::AudioData(const float* const audioData, const size_t audioDataLength)
    -> void
{
  if (not m_started)
  {
    HandleError("Goom not started - cannot process audio data.");
#ifndef GOOM_DEBUG
    return;
#endif
  }

  AddAudioDataToBuffer(std_spn::span<const float>{audioData, audioDataLength});

  if (m_audioBuffer.DataAvailable() >= m_audioSampleLen)
  {
    MoveNextAudioSampleToProducer();
  }
}

auto CVisualizationGoom::AddAudioDataToBuffer(const std_spn::span<const float>& audioData) noexcept
    -> void
{
  if (m_audioBuffer.FreeSpace() < audioData.size())
  {
    // TODO - is this a good idea?
    // Lose the audio or somehow start averaging?
    ++m_dropAudioDataNum;
#if DEBUG_LOGGING
    LogWarn(*m_goomLogger, "Not enough free space in audio buffer. Dropping this sample.");
    LogInfo(*m_goomLogger,
            "Free space = {}, this sample size = {}.",
            m_audioBuffer.FreeSpace(),
            audioData.size());
#endif
    return;
  }

#if DEBUG_LOGGING
  LogInfo(*m_goomLogger, "Adding audio data to buffer.");
  LogInfo(*m_goomLogger,
          "Audio buffer length = {}, data available = {}, free space = {}, this sample size = {}.",
          m_audioBuffer.BufferLength(),
          m_audioBuffer.DataAvailable(),
          m_audioBuffer.FreeSpace(),
          audioData.size());
#endif

  m_audioBuffer.Write(audioData);
}

auto CVisualizationGoom::MoveNextAudioSampleToProducer() noexcept -> void
{
#if DEBUG_LOGGING
  LogInfo(*m_goomLogger, "Moving audio sample to producer.");
#endif
  m_audioBuffer.Read(m_rawAudioData);

  m_goomVisualization.AddAudioSample(m_rawAudioData);
}

//-- Render -------------------------------------------------------------------
// Called once per frame. Do all rendering here.
//-----------------------------------------------------------------------------

auto CVisualizationGoom::Render() -> void
{
  if (not m_started)
  {
    LogWarn(*m_goomLogger, "CVisualizationGoom: Goom not started - skipping Render.");
    return;
  }
  if (not ReadyToRender())
  {
    return;
  }

  DoRender();
}

inline auto CVisualizationGoom::ReadyToRender() const -> bool
{
  if (static constexpr auto NUM_AUDIO_SAMPLES_BEFORE_STARTING = 6U;
      m_goomVisualization.GetNumAudioSamples() < NUM_AUDIO_SAMPLES_BEFORE_STARTING)
  {
    // Skip the first few frames - for some reason Kodi does a 'reload' before
    // starting the full music track.
    return false;
  }

  return true;
}

inline auto CVisualizationGoom::DoRender() noexcept -> void
{
  try
  {
    m_goomVisualization.GetScene().Resize(WindowDimensions{Width(), Height()});

    m_goomVisualization.GetScene().Render();
  }
  catch (const std::exception& e)
  {
    LogError(*m_goomLogger, "CVisualizationGoom: Goom render failed: {}", e.what());
  }
}

// NOLINTEND(cppcoreguidelines-pro-type-vararg, hicpp-vararg)

#ifdef __clang__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-prototypes"
#endif

ADDONCREATOR(CVisualizationGoom) // Don't touch this!

#ifdef __clang__
#pragma GCC diagnostic pop
#endif
