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
#include "gl_renderer.h"
#include "goom/compiler_versions.h"
#include "goom/goom_config.h"
#include "goom/goom_graphic.h"
#include "goom/goom_logger.h"
#include "goom/goom_utils.h"
#include "goom/sound_info.h"
#include "kodi_shader_with_effects.h"

#ifdef SAVE_AUDIO_BUFFERS
#include "src/goom/src/utils/strutils.h"
#endif

#include <array>
#include <cstddef>
#include <format>
#include <kodi/Filesystem.h>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#ifdef SAVE_AUDIO_BUFFERS
#include <filesystem>
#endif

using GOOM::AudioSamples;
using GOOM::Dimensions;
using GOOM::GetCompilerVersion;
using GOOM::GetGoomLibBuildTime;
using GOOM::GetGoomLibCompilerVersion;
using GOOM::GetGoomLibVersionInfo;
using GOOM::GlRenderer;
using GOOM::GoomControl;
using GOOM::GoomLogger;
using GOOM::GoomShaderVariables;
using GOOM::KodiShaderWithEffects;
using GOOM::PixelBuffer;
using GOOM::PixelChannelType;
using GOOM::TextureBufferDimensions;
using GOOM::WindowDimensions;
#ifdef SAVE_AUDIO_BUFFERS
using GOOM::UTILS::BufferView;
using GOOM::UTILS::FindAndReplaceAll;
#endif

#ifndef IS_KODI_BUILD
static_assert(false, "This is a Kodi file: 'IS_KODI_BUILD' should be defined.");
#endif

namespace KODI_ADDON = kodi::addon;
using AddonLogEnum   = ADDON_LOG;

static constexpr int MAX_QUALITY = 4;
static constexpr std::array<uint32_t, MAX_QUALITY + 1> WIDTHS_BY_QUALITY{
    512,
    640,
    1280,
    1600,
    1920,
};
static constexpr std::array<uint32_t, MAX_QUALITY + 1> HEIGHTS_BY_QUALITY{
    256,
    360,
    720,
    900,
    1080,
};

static constexpr const char* GOOM_ADDON_DATA_DIR =
    "special://userdata/addon_data/visualization.goom";

class CVisualizationGoom::PixelBufferGetter
{
public:
  explicit PixelBufferGetter(CVisualizationGoom& cVisualizationGoom)
    : m_cVisualizationGoom{cVisualizationGoom}
  {
  }
  auto ReserveNextActivePixelBufferData() noexcept;
  auto ReleaseActivePixelBufferData() noexcept -> void;

  [[nodiscard]] auto GetNextPixelBuffer() const noexcept -> const GOOM::PixelChannelType*;
  [[nodiscard]] auto GetNextGoomShaderVariables() const noexcept -> const GoomShaderVariables*;

private:
  CVisualizationGoom& m_cVisualizationGoom;
  CVisualizationGoom::PixelBufferData m_pixelBufferData{};
};

inline auto CVisualizationGoom::PixelBufferGetter::ReserveNextActivePixelBufferData() noexcept
{
  m_pixelBufferData = m_cVisualizationGoom.GetNextActivePixelBufferData();
}

inline auto CVisualizationGoom::PixelBufferGetter::ReleaseActivePixelBufferData() noexcept -> void
{
  if (nullptr == m_pixelBufferData.pixelBuffer)
  {
    return;
  }
  m_cVisualizationGoom.PushUsedPixels(m_pixelBufferData);
}

inline auto CVisualizationGoom::PixelBufferGetter::GetNextPixelBuffer() const noexcept
    -> const GOOM::PixelChannelType*
{
  if (nullptr == m_pixelBufferData.pixelBuffer)
  {
    return nullptr;
  }
  return reinterpret_cast<const PixelChannelType*>(m_pixelBufferData.pixelBuffer->GetIntBuff());
}

inline auto CVisualizationGoom::PixelBufferGetter::GetNextGoomShaderVariables() const noexcept
    -> const GoomShaderVariables*
{
  if (nullptr == m_pixelBufferData.pixelBuffer)
  {
    return nullptr;
  }
  return &m_pixelBufferData.goomShaderVariables;
}

CVisualizationGoom::CVisualizationGoom()
  : m_textureBufferDimensions{WIDTHS_BY_QUALITY.at(static_cast<size_t>(
                                  std::min(MAX_QUALITY, KODI_ADDON::GetSettingInt("quality")))),
                              HEIGHTS_BY_QUALITY.at(static_cast<size_t>(
                                  std::min(MAX_QUALITY, KODI_ADDON::GetSettingInt("quality"))))},
    m_goomBufferLen{
        static_cast<size_t>(m_textureBufferDimensions.width * m_textureBufferDimensions.height)},
    m_textureBufferSize{PixelBuffer::GetIntBufferSize(
        {m_textureBufferDimensions.width, m_textureBufferDimensions.height})},
    m_showTitle{static_cast<GoomControl::ShowTitleType>(KODI_ADDON::GetSettingInt("show_title"))},
    m_goomLogger{GoomControl::MakeGoomLogger()},
    m_glShader{std::make_unique<KodiShaderWithEffects>(
        *this,
        KODI_ADDON::GetAddonPath("resources/shaders/" GL_TYPE_STRING),
        glm::ortho(0.0F, static_cast<float>(Width()), 0.0F, static_cast<float>(Height())))},
    m_glRenderer{std::make_unique<GlRenderer>(
        TextureBufferDimensions{m_textureBufferDimensions.width, m_textureBufferDimensions.height},
        WindowDimensions{Width(), Height()},
        m_glShader.get(),
        *m_goomLogger)},
    m_pixelBufferGetter{std::make_unique<PixelBufferGetter>(*this)}
{
  kodi::Log(ADDON_LOG_DEBUG, "CVisualizationGoom: Created CVisualizationGoom object.");
}

//-- Destroy -------------------------------------------------------------------
// Do everything before unload of this add-on
// !!! Add-on master function !!!
//-----------------------------------------------------------------------------
CVisualizationGoom::~CVisualizationGoom()
{
  kodi::Log(ADDON_LOG_DEBUG, "CVisualizationGoom: Destroyed CVisualizationGoom object.");
  LogStop(*m_goomLogger);
}

auto CVisualizationGoom::HandleError(const std::string& errorMsg) -> void
{
  const std::string fullMsg = std20::format("CVisualizationGoom: {}", errorMsg);

#ifdef GOOM_DEBUG
  throw std::runtime_error(fullMsg);
#else
  LogError(fullMsg);
#endif
}

//-- Start --------------------------------------------------------------------
// Called when a new soundtrack is played
//-----------------------------------------------------------------------------
auto CVisualizationGoom::Start(const int numChannels,
                               const int samplesPerSec,
                               const int bitsPerSample,
                               const std::string& songName) -> bool
{
  if (m_started)
  {
    kodi::Log(ADDON_LOG_WARNING,
              "CVisualizationGoom: Already started without a stop - skipping this.");
    return true;
  }

#ifdef GOOM_DEBUG
  return StartWithNoCatch(numChannels, samplesPerSec, bitsPerSample, songName);
#else
  return StartWithCatch(numChannels, samplesPerSec, bitsPerSample, songName);
#endif
}

inline auto CVisualizationGoom::StartWithNoCatch(const int numChannels,
                                                 const int samplesPerSec,
                                                 const int bitsPerSample,
                                                 const std::string& songName) -> bool
{
  return StartVis(numChannels, samplesPerSec, bitsPerSample, songName);
}

auto CVisualizationGoom::StartWithCatch(const int numChannels,
                                        const int samplesPerSec,
                                        const int bitsPerSample,
                                        const std::string& songName) -> bool
{
  try
  {
    return StartVis(numChannels, samplesPerSec, bitsPerSample, songName);
  }
  catch (const std::exception& e)
  {
    HandleError(std20::format("Goom start failed: {}", e.what()));
    return false;
  }
}

auto CVisualizationGoom::StartVis(const int numChannels,
                                  [[maybe_unused]] const int samplesPerSec,
                                  [[maybe_unused]] const int bitsPerSample,
                                  [[maybe_unused]] const std::string& songName) -> bool
{
  StartLogging();

  LogInfo(*m_goomLogger,
          "CVisualizationGoom: Texture width, height = {}, {}.",
          m_textureBufferDimensions.width,
          m_textureBufferDimensions.height);
#ifdef HAS_GL
  LogInfo(*m_goomLogger,
          "CVisualizationGoom: Supported GLSL version is {}.",
          reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION)));
#endif

  SetNumChannels(numChannels);
  StartActivePixelBufferDataQueue();

  m_glRenderer->Start();
  InitGoomController();

  m_started = true;

  return true;
}

auto CVisualizationGoom::SetNumChannels(const int numChannels) -> void
{
  m_numChannels    = static_cast<size_t>(numChannels);
  m_audioBufferLen = m_numChannels * AudioSamples::AUDIO_SAMPLE_LEN;
  m_audioBufferNum = 0;
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

auto CVisualizationGoom::StartActivePixelBufferDataQueue() -> void
{
  // Make one initial frame in black
  const PixelBufferData initialBufferData{MakePixelBufferData()};
  initialBufferData.pixelBuffer->Fill(GOOM::BLACK_PIXEL);
  m_activePixelBufferDataQueue.push(initialBufferData);
}

inline auto CVisualizationGoom::MakePixelBufferData() const -> PixelBufferData
{
  PixelBufferData pixelBufferData;
  pixelBufferData.pixelBuffer = std::make_shared<PixelBuffer>(
      Dimensions{m_textureBufferDimensions.width, m_textureBufferDimensions.height});
  return pixelBufferData;
}

//-- Stop ---------------------------------------------------------------------
// Called when the visualisation is closed by Kodi
//-----------------------------------------------------------------------------
auto CVisualizationGoom::Stop() -> void
{
  if (!m_started)
  {
    LogWarn(*m_goomLogger, "CVisualizationGoom: Not started - skipping this.");
    return;
  }

#ifdef GOOM_DEBUG
  StopWithNoCatch();
#else
  StopWithCatch();
#endif
}

inline auto CVisualizationGoom::StopWithNoCatch() -> void
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
    HandleError(std20::format("Goom stop failed: {}", e.what()));
  }
}

inline auto CVisualizationGoom::StopVis() -> void
{
  LogInfo(*m_goomLogger, "CVisualizationGoom: Visualization stopping.");
  m_started = false;

  StopGoomProcessBuffersThread();

  DeinitGoomController();

  m_glRenderer->Stop();

  m_audioBufferNum = 0;

  LogStop(*m_goomLogger);
}

inline auto GetGoomVisualizationBuildTime() -> std::string
{
  return GetBuildTime();
}

auto CVisualizationGoom::InitGoomController() -> void
{
  if (m_goomControl)
  {
    throw std::runtime_error("CVisualizationGoom: Goom controller already initialized!");
  }

  LogInfo(*m_goomLogger, "CVisualizationGoom: Initializing goom controller.");
  m_goomControl = std::make_unique<GoomControl>(
      Dimensions{m_textureBufferDimensions.width, m_textureBufferDimensions.height},
      KODI_ADDON::GetAddonPath("resources"),
      *m_goomLogger);

  LogInfo(*m_goomLogger, "CVisualizationGoom: Goom: {}.", GetGoomLibVersionInfo());
  LogInfo(*m_goomLogger, "CVisualizationGoom: Compiler: {}.", GetCompilerVersion());
  LogInfo(*m_goomLogger, "CVisualizationGoom: Build Time: {}.", GetGoomVisualizationBuildTime());
  LogInfo(*m_goomLogger, "Goom Library: Compiler: {}.", GetGoomLibCompilerVersion());
  LogInfo(*m_goomLogger, "Goom Library: Build Time: {}.", GetGoomLibBuildTime());

  m_goomControl->SetShowGoomState(KODI_ADDON::GetSettingBoolean("show_goom_state"));
  m_goomControl->SetDumpDirectory(kodi::vfs::TranslateSpecialProtocol(
      std::string(GOOM_ADDON_DATA_DIR) + GOOM::PATH_SEP + "goom_dumps"));
  m_goomControl->SetShowTitle(m_showTitle);

  // Goom will use same random sequence each time if the following is uncommented:
  //GoomControl::SetRandSeed(1);
  m_goomControl->Start();
}

auto CVisualizationGoom::DeinitGoomController() -> void
{
  if (!m_goomControl)
  {
    HandleError("Goom controller already uninitialized!");
    return;
  }

  m_goomControl->Finish();
  m_goomControl.reset();
  LogInfo(*m_goomLogger, "CVisualizationGoom: Uninitialized goom controller.");
}

auto CVisualizationGoom::UpdateTrack(const kodi::addon::VisualizationTrack& track) -> bool
{
  if (!m_goomControl)
  {
    return true;
  }

  const std::string artist =
      !track.GetArtist().empty() ? track.GetArtist() : track.GetAlbumArtist();
  const std::string currentSongName =
      artist.empty() ? track.GetTitle() : artist + " - " + track.GetTitle();

  LogInfo(*m_goomLogger, "Current Song Title = '{}'", currentSongName);
  LogInfo(*m_goomLogger, "Genre = '{}', Duration = {}", track.GetGenre(), track.GetDuration());

  m_goomControl->SetSongInfo(
      {currentSongName, track.GetGenre(), static_cast<uint32_t>(track.GetDuration())});

#ifdef SAVE_AUDIO_BUFFERS
  m_audioBufferWriter = GetAudioBufferWriter(currentSongName);
#endif

  return true;
}

auto CVisualizationGoom::StartGoomProcessBuffersThread() -> void
{

  LogInfo(*m_goomLogger, "CVisualizationGoom: Starting goom process buffers thread.");
  m_workerThread = std::thread(&CVisualizationGoom::Process, this);
}

auto CVisualizationGoom::StopGoomProcessBuffersThread() -> void
{
  LogInfo(*m_goomLogger, "CVisualizationGoom: Stopping goom process buffers thread.");

  ExitWorkerThread();

  if (m_workerThread.joinable())
  {
    m_workerThread.join();
  }
  LogInfo(*m_goomLogger, "CVisualizationGoom: Goom process buffers thread stopped.");
}

auto CVisualizationGoom::ExitWorkerThread() -> void
{
  const std::unique_lock lock(m_mutex);
  m_workerThreadExit = true;
  m_wait.notify_one();
}

//-- AudioData ----------------------------------------------------------------
// Called by Kodi to pass new audio data to the vis
//-----------------------------------------------------------------------------
auto CVisualizationGoom::AudioData(const float* const audioData, const size_t audioDataLength)
    -> void
{
  if (!m_started)
  {
    HandleError("Goom not started - cannot process audio.");
    return;
  }

  ++m_audioBufferNum;

  if (1 == m_audioBufferNum)
  {
    LogInfo(*m_goomLogger, "CVisualizationGoom: Starting audio data processing.");
    StartGoomProcessBuffersThread();
  }

  const std::unique_lock lock(m_mutex);
  if (m_audioBuffer.DataAvailable() >= CIRCULAR_BUFFER_SIZE)
  {
    AudioDataQueueTooBig();
    return;
  }

  m_audioBuffer.Write(audioData, static_cast<size_t>(audioDataLength));
  m_wait.notify_one();
}

inline auto CVisualizationGoom::GetNextActivePixelBufferData() -> PixelBufferData
{
  PixelBufferData pixelBufferData{};

  const std::scoped_lock lk(m_mutex);
  if (m_activePixelBufferDataQueue.empty())
  {
    NoActiveBufferAvailable();
  }
  else
  {
    pixelBufferData = m_activePixelBufferDataQueue.front();
    m_activePixelBufferDataQueue.pop();
  }

  return pixelBufferData;
}

inline auto CVisualizationGoom::PushUsedPixels(const PixelBufferData& pixelBufferData) -> void
{
  const std::scoped_lock lk(m_mutex);
  m_storedPixelBufferDataQueue.push(pixelBufferData);
}

inline auto CVisualizationGoom::SetNextRenderData() noexcept -> void
{
  const auto* const pixelBuffer = m_pixelBufferGetter->GetNextPixelBuffer();
  m_glRenderer->SetPixelBuffer(pixelBuffer);
  if (pixelBuffer != nullptr)
  {
    m_glShader->SetShaderVariables(*m_pixelBufferGetter->GetNextGoomShaderVariables());
  }
}

auto CVisualizationGoom::Process() -> void
{
#ifdef GOOM_DEBUG
  ProcessWithNoCatch();
#else
  ProcessWithCatch();
#endif
}

inline auto CVisualizationGoom::ProcessWithNoCatch() -> void
{
  ProcessVis();
}

inline auto CVisualizationGoom::ProcessWithCatch() -> void
{
  try
  {
    ProcessVis();
  }
  catch (const std::exception& e)
  {
    HandleError(std20::format("Goom process failed: {}", e.what()));
  }
}

auto CVisualizationGoom::ProcessVis() -> void
{
  std::vector<float> floatAudioData(m_audioBufferLen);

  while (true)
  {
    std::unique_lock lk(m_mutex);
    if (m_workerThreadExit)
    {
      break;
    }

    if (m_audioBuffer.DataAvailable() < m_audioBufferLen)
    {
      m_wait.wait(lk);
    }
    if (const size_t read = m_audioBuffer.Read(floatAudioData.data(), m_audioBufferLen);
        read != m_audioBufferLen)
    {
      LogWarn(*m_goomLogger,
              "CVisualizationGoom: Num read audio length {} != {} = expected audio data length - "
              "skipping this.",
              read,
              m_audioBufferLen);
      AudioDataIncorrectReadLength();
      continue;
    }
    lk.unlock();

    if (m_workerThreadExit)
    {
      break;
    }

    lk.lock();
    if (m_activePixelBufferDataQueue.size() > MAX_ACTIVE_QUEUE_LENGTH)
    {
      // Too far behind, skip this audio data.
      SkippedAudioData();
      continue;
    }
    lk.unlock();

    PixelBufferData pixelBufferData;
    lk.lock();
    if (m_storedPixelBufferDataQueue.empty())
    {
      pixelBufferData = MakePixelBufferData();
    }
    else
    {
      pixelBufferData = m_storedPixelBufferDataQueue.front();
      m_storedPixelBufferDataQueue.pop();
    }
    lk.unlock();

    UpdateGoomBuffer(floatAudioData, pixelBufferData);

    lk.lock();
    m_activePixelBufferDataQueue.push(pixelBufferData);
    lk.unlock();
  }
}

inline auto CVisualizationGoom::UpdateGoomBuffer(const std::vector<float>& floatAudioData,
                                                 PixelBufferData& pixelBufferData) -> void
{
  const AudioSamples audioData{m_numChannels, floatAudioData};
  m_goomControl->SetGoomBuffer(pixelBufferData.pixelBuffer);
  m_goomControl->UpdateGoomBuffer(audioData);
  pixelBufferData.goomShaderVariables = m_goomControl->GetLastShaderVariables();

#ifdef SAVE_AUDIO_BUFFERS
  SaveAudioBuffer(floatAudioData);
#endif
}

//-- Render -------------------------------------------------------------------
// Called once per frame. Do all rendering here.
//-----------------------------------------------------------------------------

auto CVisualizationGoom::Render() -> void
{
  if (not m_started)
  {
    LogWarn(*m_goomLogger, "CVisualizationGoom: Goom not started - skipping this.");
    return;
  }
  if (m_audioBufferNum < MIN_AUDIO_BUFFERS_BEFORE_STARTING)
  {
    // Skip the first few - for some reason Kodi does a 'reload'
    // before really starting the music.
    return;
  }

  m_pixelBufferGetter->ReserveNextActivePixelBufferData();

  try
  {
    DoRender();
  }
  catch (const std::exception& e)
  {
    LogError(*m_goomLogger, "CVisualizationGoom: Goom render failed: {}", e.what());
  }

  m_pixelBufferGetter->ReleaseActivePixelBufferData();
}

inline auto CVisualizationGoom::DoRender() noexcept -> void
{
  SetNextRenderData();
  m_glRenderer->Render();
}

#ifdef SAVE_AUDIO_BUFFERS

[[nodiscard]] auto ReplaceIllegalFilenameChars(const std::string& str) noexcept -> std::string
{
  std::string legalStr = str;

  FindAndReplaceAll(legalStr, " - ", "-");
  FindAndReplaceAll(legalStr, "- ", "-");
  FindAndReplaceAll(legalStr, " -", "-");
  FindAndReplaceAll(legalStr, " ", "_");
  FindAndReplaceAll(legalStr, "&", "and");
  FindAndReplaceAll(legalStr, "/", "-");
  FindAndReplaceAll(legalStr, "\\", "-");
  FindAndReplaceAll(legalStr, "?", "");
  FindAndReplaceAll(legalStr, ":", "-");
  FindAndReplaceAll(legalStr, "|", "-");
  FindAndReplaceAll(legalStr, ">", "-");
  FindAndReplaceAll(legalStr, "<", "-");
  FindAndReplaceAll(legalStr, "\"", "");
  FindAndReplaceAll(legalStr, "'", "");

  return legalStr;
}

auto CVisualizationGoom::GetAudioBufferWriter(const std::string& songName)
    -> std::unique_ptr<AudioBufferWriter>
{
  const std::string filename = ReplaceIllegalFilenameChars(songName);

  static constexpr const char* AUDIO_OUTPUT_FILE_PREFIX = "audio_buffers";
  const std::string kodiGoomDataDir = kodi::vfs::TranslateSpecialProtocol(GOOM_ADDON_DATA_DIR);
  const std::string saveDirectory   = kodiGoomDataDir + GOOM::PATH_SEP + AUDIO_OUTPUT_FILE_PREFIX +
                                    GOOM::PATH_SEP + filename + GOOM::PATH_SEP + "audio";
  if (std::filesystem::exists(saveDirectory))
  {
    std::filesystem::remove_all(saveDirectory);
  }
  if (not std::filesystem::create_directories(saveDirectory))
  {
    throw std::runtime_error{std20::format("Could not create directory '{}'.", saveDirectory)};
  }

  const std::string saveFilePrefix = saveDirectory + GOOM::PATH_SEP + "audio_buffer";

  return std::make_unique<AudioBufferWriter>(saveFilePrefix, true);
}

auto CVisualizationGoom::SaveAudioBuffer(const std::vector<float>& floatAudioData) -> void
{
  if (m_audioBufferWriter == nullptr)
  {
    return;
  }

  const BufferView<float> audioBuffer{floatAudioData.size(),
                                      const_cast<float*>(floatAudioData.data())};
  const int64_t bufferNum = m_audioBufferWriter->GetCurrentBufferNum();
  /**
  if (bufferNum == 0) {
    for (size_t i = 0; i < floatAudioData.size(); ++i)
    {
      LogInfo("floatAudioData[{}] = {}", i, floatAudioData[i]);
    }
  }
   **/

  const std::string currentFilename = m_audioBufferWriter->GetCurrentFilename();
  m_audioBufferWriter->Write(audioBuffer, true);
  LogInfo("Wrote audio data buffer {} to file '{}'.", bufferNum, currentFilename);
}

#endif

ADDONCREATOR(CVisualizationGoom) // Don't touch this!
