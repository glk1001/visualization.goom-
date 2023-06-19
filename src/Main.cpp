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
#include "goom/goom_config.h"
#include "goom/goom_control.h"
#include "goom/goom_graphic.h"
#include "goom/goom_logger.h"
#include "goom_buffer_producer.h"

#include <array>
#include <format>
#include <kodi/Filesystem.h>
#include <memory>
#include <span>
#include <string>

using GOOM::GlRenderer;
using GOOM::GoomBufferProducer;
using GOOM::GoomControl;
using GOOM::GoomLogger;
using GOOM::GoomShaderVariables;
using GOOM::PixelChannelType;
using GOOM::TextureBufferDimensions;
using GOOM::WindowDimensions;

#ifndef IS_KODI_BUILD
static_assert(false, "This is a Kodi file: 'IS_KODI_BUILD' should be defined.");
#endif

using PixelBufferData = GOOM::GoomBufferProducer::PixelBufferData;

namespace KODI_ADDON = kodi::addon;
using AddonLogEnum   = ADDON_LOG;

class CVisualizationGoom::PixelBufferGetter
{
public:
  explicit PixelBufferGetter(GoomBufferProducer& bufferProducer);

  auto ReserveNextActivePixelBufferData() noexcept;
  auto ReleaseActivePixelBufferData() noexcept -> void;

  [[nodiscard]] auto GetNextPixelBuffer() const noexcept -> const PixelChannelType*;
  [[nodiscard]] auto GetNextGoomShaderVariables() const noexcept -> const GoomShaderVariables*;

private:
  GoomBufferProducer* m_bufferProducer;
  PixelBufferData m_pixelBufferData{};
};

inline CVisualizationGoom::PixelBufferGetter::PixelBufferGetter(GoomBufferProducer& bufferProducer)
  : m_bufferProducer{&bufferProducer}
{
}

inline auto CVisualizationGoom::PixelBufferGetter::ReserveNextActivePixelBufferData() noexcept
{
  m_pixelBufferData = m_bufferProducer->GetNextActivePixelBufferData();
}

inline auto CVisualizationGoom::PixelBufferGetter::ReleaseActivePixelBufferData() noexcept -> void
{
  if (nullptr == m_pixelBufferData.pixelBuffer)
  {
    return;
  }
  m_bufferProducer->PushUsedPixels(m_pixelBufferData);
}

inline auto CVisualizationGoom::PixelBufferGetter::GetNextPixelBuffer() const noexcept
    -> const PixelChannelType*
{
  if (nullptr == m_pixelBufferData.pixelBuffer)
  {
    return nullptr;
  }
  return m_pixelBufferData.pixelBuffer->GetBuffPtr();
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

} // namespace

CVisualizationGoom::CVisualizationGoom()
  : m_goomLogger{GoomControl::MakeGoomLogger()},
    m_glRenderer{KODI_ADDON::GetAddonPath(std::string{SHADERS_DIR} + PATH_SEP + GL_TYPE_STRING),
                 GetTextureBufferDimensions(),
                 WindowDimensions{Width(), Height()},
                 *m_goomLogger},
    m_goomBufferProducer{
        GetTextureBufferDimensions(),
        KODI_ADDON::GetAddonPath(RESOURCES_DIR),
        static_cast<GoomControl::ShowMusicTitleType>(KODI_ADDON::GetSettingInt(SHOW_TITLE_SETTING)),
#ifdef SAVE_AUDIO_BUFFERS
        GetAudioBuffersSaveDir(),
#endif
        *m_goomLogger},
    m_pixelBufferGetter{std::make_unique<PixelBufferGetter>(m_goomBufferProducer)}
{
  StartLogging();
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg, hicpp-vararg)
  kodi::Log(ADDON_LOG_DEBUG, "CVisualizationGoom: Created CVisualizationGoom object.");
}

//-- Destroy -------------------------------------------------------------------
// Do everything before unload of this add-on
// !!! Add-on master function !!!
//-----------------------------------------------------------------------------
// NOLINTBEGIN(clang-analyzer-optin.cplusplus.VirtualCall)
CVisualizationGoom::~CVisualizationGoom()
{
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg, hicpp-vararg)
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
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg, hicpp-vararg)
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
  LogInfo(*m_goomLogger, "CVisualizationGoom: Build Time: {}.", GetGoomVisualizationBuildTime());

  m_goomBufferProducer.SetShowGoomState(KODI_ADDON::GetSettingBoolean(SHOW_GOOM_STATE_SETTING));
  m_goomBufferProducer.SetDumpDirectory(kodi::vfs::TranslateSpecialProtocol(
      std::string(GOOM_ADDON_DATA_DIR) + PATH_SEP + GOOM_DUMPS_SETTING));

  m_goomBufferProducer.SetNumChannels(static_cast<uint32_t>(numChannels));
  m_goomBufferProducer.Start();

  m_glRenderer.Init();

  LogInfo(*m_goomLogger, "Starting process Goom buffers thread.");
  m_processBuffersThread =
      std::thread{&GoomBufferProducer::ProcessGoomBuffersThread, &m_goomBufferProducer};

  m_started = true;
}

inline auto CVisualizationGoom::StopVis() -> void
{
  LogInfo(*m_goomLogger, "CVisualizationGoom: Visualization stopping.");
  m_started = false;

  m_goomBufferProducer.Stop();

  m_glRenderer.Destroy();

  LogInfo(*m_goomLogger, "Stopping process Goom buffers thread.");
  m_processBuffersThread.join();

  LogStop(*m_goomLogger);
}

auto CVisualizationGoom::StartLogging() -> void
{
  static const auto s_KODI_LOGGER = [](const GoomLogger::LogLevel lvl, const std::string& msg)
  {
    const auto kodiLvl = static_cast<AddonLogEnum>(static_cast<size_t>(lvl));
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg, hicpp-vararg)
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

  m_goomBufferProducer.UpdateTrack(
      {currentSongName, track.GetGenre(), static_cast<uint32_t>(track.GetDuration())});

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

  m_goomBufferProducer.ProcessAudioData(std_spn::span<const float>{audioData, audioDataLength});
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
  if (static constexpr auto MIN_AUDIO_BUFFERS_BEFORE_STARTING = 6U;
      m_goomBufferProducer.GetAudioBufferNum() < MIN_AUDIO_BUFFERS_BEFORE_STARTING)
  {
    // Skip the first few frames - for some reason Kodi does a 'reload' before
    // starting the full music track.
    return false;
  }

  return true;
}

inline auto CVisualizationGoom::DoRender() noexcept -> void
{
  m_pixelBufferGetter->ReserveNextActivePixelBufferData();

  try
  {
    const auto* const pixelBuffer = m_pixelBufferGetter->GetNextPixelBuffer();
    m_glRenderer.SetPixelBuffer(pixelBuffer);
    if (pixelBuffer != nullptr)
    {
      m_glRenderer.SetShaderVariables(*m_pixelBufferGetter->GetNextGoomShaderVariables());
    }

    m_glRenderer.Render();
  }
  catch (const std::exception& e)
  {
    LogError(*m_goomLogger, "CVisualizationGoom: Goom render failed: {}", e.what());
  }

  m_pixelBufferGetter->ReleaseActivePixelBufferData();
}

#ifdef __clang__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-prototypes"
#endif

ADDONCREATOR(CVisualizationGoom) // Don't touch this!

#ifdef __clang__
#pragma GCC diagnostic pop
#endif
