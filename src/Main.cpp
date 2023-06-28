/*
 *  Copyright (C) 2005-2022 Team Kodi (https://kodi.tv)
 *  Copyright (C) 2005-2013 Team XBMC
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#undef NO_LOGGING

#include "Main.h"

#include "build_time.h"
#include "displacement_filter.h"
#include "goom/compiler_versions.h"
#include "goom/goom_config.h"
#include "goom/goom_control.h"
#include "goom/goom_graphic.h"
#include "goom/goom_logger.h"
#include "slot_producer_consumer.h"

#include <algorithm>
#include <array>
#include <format>
#include <kodi/Filesystem.h>
#include <memory>
#include <random>
#include <span>
#include <stdexcept>
#include <string>

using GOOM::AudioSamples;
using GOOM::Dimensions;
using GOOM::GoomControl;
using GOOM::GoomLogger;
using GOOM::SetRandSeed;
using GOOM::SlotProducerConsumer;
using GOOM::TextureBufferDimensions;
using GOOM::WindowDimensions;
using GOOM::OPENGL::DisplacementFilter;

#ifndef IS_KODI_BUILD
static_assert(false, "This is a Kodi file: 'IS_KODI_BUILD' should be defined.");
#endif

namespace KODI_ADDON = kodi::addon;
using AddonLogEnum   = ADDON_LOG;

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

constexpr auto* QUALITY_SETTING = "quality";
//TODO - constexpr auto* SHOW_TITLE_SETTING      = "show_title";
constexpr auto* SHOW_GOOM_STATE_SETTING = "show_goom_state";
constexpr auto* GOOM_DUMPS_SETTING      = "goom_dumps";

[[nodiscard]] inline auto GetTextureBufferDimensions() noexcept -> TextureBufferDimensions
{
  return {WIDTHS_BY_QUALITY.at(static_cast<size_t>(
              std::min(MAX_QUALITY, KODI_ADDON::GetSettingInt(QUALITY_SETTING)))),
          HEIGHTS_BY_QUALITY.at(static_cast<size_t>(
              std::min(MAX_QUALITY, KODI_ADDON::GetSettingInt(QUALITY_SETTING))))};
}

[[nodiscard]] inline auto GetGoomVisualizationBuildTime() -> std::string
{
  return ::GetBuildTime();
}

#ifdef SAVE_AUDIO_BUFFERS
[[nodiscard]] auto GetAudioBuffersSaveDir()
{
  static constexpr auto* AUDIO_BUFFERS_DIR_PREFIX = "audio_buffers";

  const auto kodiGoomDataDir = kodi::vfs::TranslateSpecialProtocol(GOOM_ADDON_DATA_DIR);

  return kodiGoomDataDir + GOOM::PATH_SEP + AUDIO_BUFFERS_DIR_PREFIX;
}
#endif

constexpr auto MAX_BUFFER_QUEUE_LEN     = DisplacementFilter::NUM_PBOS;
constexpr auto MAX_AUDIO_DATA_QUEUE_LEN = 100U;

} // namespace

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, hicpp-vararg)

CVisualizationGoom::CVisualizationGoom()
  : m_goomLogger{GoomControl::MakeGoomLogger()},
    m_glScene{KODI_ADDON::GetAddonPath(std::string{SHADERS_DIR} + PATH_SEP + GL_TYPE_STRING),
              GetTextureBufferDimensions()},
    m_goomControl{std::make_unique<GoomControl>(
        Dimensions{GetTextureBufferDimensions().width, GetTextureBufferDimensions().height},
        KODI_ADDON::GetAddonPath(RESOURCES_DIR),
        *m_goomLogger)},
    m_slotProducerConsumer{*m_goomLogger, MAX_BUFFER_QUEUE_LEN, MAX_AUDIO_DATA_QUEUE_LEN}
{
  m_glScene.Resize(WindowDimensions{Width(), Height()});

  m_slotProducerConsumer.SetProduceItem([this](const size_t slot, const AudioSamples& audioSamples)
                                        { ProduceItem(slot, audioSamples); });
  m_slotProducerConsumer.SetConsumeItem([this](const size_t slot) { ConsumeItem(slot); });

  auto requestNextDataFrame = [this]() { m_slotProducerConsumer.Consume(); };
  m_glScene.SetRequestNextFrameDataFunc(requestNextDataFrame);

  StartLogging();
  kodi::Log(ADDON_LOG_DEBUG, "CVisualizationGoom: Created CVisualizationGoom object.");
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
  LogInfo(*m_goomLogger, "Starting visualization.");

  Expects(m_goomControl != nullptr);
  Expects(not m_started);

  LogInfo(*m_goomLogger, "CVisualizationGoom: Build Time: {}.", GetGoomVisualizationBuildTime());
  LogInfo(*m_goomLogger, "Goom: Version: {}.", GOOM::GetGoomLibVersionInfo());
  LogInfo(*m_goomLogger, "Goom: Compiler: {}.", GOOM::GetCompilerVersion());
  LogInfo(*m_goomLogger, "Goom Library: Compiler: {}.", GOOM::GetGoomLibCompilerVersion());
  LogInfo(*m_goomLogger, "Goom Library: Build Time: {}.", GOOM::GetGoomLibBuildTime());

  LogInfo(*m_goomLogger,
          "Texture width, height = {}, {}.",
          GetTextureBufferDimensions().width,
          GetTextureBufferDimensions().height);
  LogInfo(*m_goomLogger,
          "Window width, height, pixel ratio = {}, {}, {}.",
          Width(),
          Height(),
          PixelRatio());
  LogInfo(*m_goomLogger,
          "Scene frame width, height = {}, {}.",
          m_glScene.GetFramebufferWidth(),
          m_glScene.GetFramebufferHeight());

  InitAudioData(numChannels);
  InitSceneFrameData();
  InitGoomControl();

  m_goomControl->Start();
  m_slotProducerConsumer.Start();

  LogInfo(*m_goomLogger, "Starting slot producer consumer thread.");
  m_slotProducerConsumerThread =
      std::thread{&SlotProducerConsumer<AudioSamples>::ProducerThread, &m_slotProducerConsumer};

  m_started = true;
}

inline auto CVisualizationGoom::StopVis() -> void
{
  LogInfo(*m_goomLogger, "CVisualizationGoom: Visualization stopping.");
  m_started = false;

  m_slotProducerConsumer.Stop();
  m_glScene.DestroyScene();

  LogInfo(*m_goomLogger, "Stopping slot producer consumer thread.");
  m_slotProducerConsumerThread.join();

  LogStop(*m_goomLogger);
}

auto CVisualizationGoom::InitAudioData(const int32_t numChannels) noexcept -> void
{
  m_numChannels    = static_cast<size_t>(numChannels);
  m_audioSampleLen = m_numChannels * AudioSamples::AUDIO_SAMPLE_LEN;
  m_rawAudioData.resize(m_audioSampleLen);
  m_audioBuffer.Clear();
}

auto CVisualizationGoom::InitSceneFrameData() noexcept -> void
{
  m_glScene.InitScene();
  m_goomControl->InitFrameData(m_glScene.GetFrameDataArray());
  m_glScene.InitAllFrameDataToGl();
}

auto CVisualizationGoom::InitGoomControl() noexcept -> void
{
  SetRandSeed(std::random_device{}());

  m_goomControl->SetShowGoomState(KODI_ADDON::GetSettingBoolean(SHOW_GOOM_STATE_SETTING));
  m_goomControl->SetDumpDirectory(kodi::vfs::TranslateSpecialProtocol(
      std::string(GOOM_ADDON_DATA_DIR) + PATH_SEP + GOOM_DUMPS_SETTING));

  m_goomControl->SetFrameData(m_glScene.GetFrameData(0));
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
  const auto artist = not track.GetArtist().empty() ? track.GetArtist() : track.GetAlbumArtist();
  const auto currentSongName =
      artist.empty() ? track.GetTitle() : ((artist + " - ") + track.GetTitle());

  LogInfo(*m_goomLogger, "Current Title = '{}'", currentSongName);
  LogInfo(*m_goomLogger, "Genre = '{}', Duration = {}", track.GetGenre(), track.GetDuration());

  m_goomControl->SetSongInfo(
      {currentSongName, track.GetGenre(), static_cast<uint32_t>(track.GetDuration())});

#ifdef SAVE_AUDIO_BUFFERS
  m_audioBufferWriter = GetAudioBufferWriter(track.title);
#endif

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

  // LogInfo(*m_goomLogger, "Adding audio data to circular buffer.");
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
    // Lose the audio?????
    return;
  }

  m_audioBuffer.Write(audioData);
}

auto CVisualizationGoom::MoveNextAudioSampleToProducer() noexcept -> void
{
  // LogInfo(*m_goomLogger, "Moving audio sample to producer.");
  m_audioBuffer.Read(m_rawAudioData);
  m_slotProducerConsumer.AddResource(AudioSamples{m_numChannels, m_rawAudioData});
  ++m_audioSamplesNum;
}

auto CVisualizationGoom::ConsumeItem(const size_t slot) noexcept -> void
{
  // LogInfo(*m_goomLogger, std_fmt::format("Consumer consuming slot {}.", slot));
  m_glScene.UpdateFrameData(slot);
}

auto CVisualizationGoom::ProduceItem(const size_t slot, const AudioSamples& audioSamples) noexcept
    -> void
{
  // LogInfo(*m_goomLogger, std_fmt::format("Producer producing slot {}.", slot));

  auto& frameData = m_glScene.GetFrameData(slot);

  m_goomControl->SetFrameData(frameData);
  m_goomControl->UpdateGoomBuffers(audioSamples);

  const auto shaderVariables    = m_goomControl->GetLastShaderVariables();
  frameData.miscData.brightness = shaderVariables.brightness;

#ifdef SAVE_AUDIO_BUFFERS
  SaveAudioBuffer(audioSamples);
#endif
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
      m_audioSamplesNum < NUM_AUDIO_SAMPLES_BEFORE_STARTING)
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
    m_glScene.Resize(WindowDimensions{Width(), Height()});

    m_glScene.Render();
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
