#undef NO_LOGGING

#include "goom_visualization.h"

#include "build_time.h"
#include "displacement_filter.h"
#include "goom/compiler_versions.h"
#include "goom/goom_control.h"
#include "slot_producer_consumer.h"

#include <chrono>
#include <thread>

namespace GOOM::VIS
{

using GOOM::AudioSamples;
using GOOM::Dimensions;
using GOOM::GoomControl;
using GOOM::SetRandSeed;
using GOOM::ShowSongTitleType;
using GOOM::SlotProducerConsumer;
using GOOM::TextureBufferDimensions;
using GOOM::WindowDimensions;
using GOOM::OPENGL::DisplacementFilter;

#define DEBUG_LOGGING 0
//#define DEBUG_LOGGING 1

namespace
{

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

GoomVisualization::GoomVisualization(GOOM::GoomLogger& goomLogger,
                                     const std::string& resourcesDir,
                                     const std::string& shaderDir,
                                     const TextureBufferDimensions& textureBufferDimensions)
  : m_goomLogger{&goomLogger},
    m_glScene{shaderDir, textureBufferDimensions},
    m_goomControl{std::make_unique<GoomControl>(
        Dimensions{textureBufferDimensions.width, textureBufferDimensions.height},
        resourcesDir,
        *m_goomLogger)},
    m_slotProducerConsumer{*m_goomLogger, MAX_BUFFER_QUEUE_LEN, MAX_AUDIO_DATA_QUEUE_LEN}
{
  m_slotProducerConsumer.SetProduceItem([this](const size_t slot, const AudioSamples& audioSamples)
                                        { ProduceItem(slot, audioSamples); });
  m_slotProducerConsumer.SetConsumeItem([this](const size_t slot) { ConsumeItem(slot); });

  static constexpr auto CONSUME_WAIT_FOR_MS = 1U;
  auto requestNextDataFrame                 = [this]()
  { return m_slotProducerConsumer.ConsumeWithoutRelease(CONSUME_WAIT_FOR_MS); };
  m_glScene.SetRequestNextFrameDataFunc(requestNextDataFrame);

  auto releaseCurrentFrameData = [this](const size_t slot)
  { m_slotProducerConsumer.Release(slot); };
  m_glScene.SetReleaseCurrentFrameDataFunc(releaseCurrentFrameData);

  LogDebug(*m_goomLogger, "Created Goom visualizationGoom object.");
}

GoomVisualization::~GoomVisualization() noexcept = default;

auto GoomVisualization::SetRandomSeed(const uint64_t seed) noexcept -> void
{
  SetRandSeed(seed);
}

auto GoomVisualization::SetWindowDimensions(const WindowDimensions& windowDimensions) noexcept
    -> void
{
  m_glScene.Resize(windowDimensions);
}

auto GoomVisualization::SetShowSongTitle(ShowSongTitleType showMusicTitleType) -> void
{
  m_goomControl->SetShowSongTitle(showMusicTitleType);
}

auto GoomVisualization::SetShowGoomState(const bool value) -> void
{
  m_goomControl->SetShowGoomState(value);
}

auto GoomVisualization::SetDumpDirectory(const std::string& dumpDirectory) -> void
{
  m_goomControl->SetDumpDirectory(dumpDirectory);
}

auto GoomVisualization::Start(const int numChannels) -> void
{
  Expects(not m_started);

  LogInfo(*m_goomLogger, "Starting visualization.");

  Expects(m_goomControl != nullptr);
  Expects(not m_started);

  LogInfo(*m_goomLogger, "Goom Vis: Build Time     : {}.", GetGoomVisualizationBuildTime());
  LogInfo(*m_goomLogger, "Goom: Version            : {}.", GOOM::GetGoomLibVersionInfo());
  LogInfo(*m_goomLogger, "Goom: Compiler           : {}.", GOOM::GetCompilerVersion());
  LogInfo(*m_goomLogger, "Goom Library: Compiler   : {}.", GOOM::GetGoomLibCompilerVersion());
  LogInfo(*m_goomLogger, "Goom Library: Build Time : {}.", GOOM::GetGoomLibBuildTime());
  LogInfo(*m_goomLogger, "Random seed              : {}.", GetRandSeed());
  LogInfo(*m_goomLogger, "Num pool threads         : {}.", m_goomControl->GetNumPoolThreads());

  LogInfo(*m_goomLogger,
          "Texture width, height = {}, {}.",
          m_glScene.GetWidth(),
          m_glScene.GetHeight());
  LogInfo(*m_goomLogger,
          "Scene frame width, height = {}, {}.",
          m_glScene.GetFramebufferWidth(),
          m_glScene.GetFramebufferHeight());

  InitAudioValues(numChannels);
  InitSceneFrameData();
  InitGoomControl();

  m_goomControl->Start();
  m_slotProducerConsumer.Start();

  m_totalProductionTimeInMs = 0.0;
  m_numItemsProduced        = 0U;

  m_started = true;
}

auto GoomVisualization::StartThread() -> void
{
  Expects(m_started);

  LogInfo(*m_goomLogger, "Slot producer consumer thread starting.");
  m_slotProducerConsumerThread =
      std::thread{&SlotProducerConsumer<AudioSamples>::ProducerThread, &m_slotProducerConsumer};
}

auto GoomVisualization::Stop() -> void
{
  Expects(m_started);

  m_started = false;

  LogInfo(*m_goomLogger, "Goom visualization stopping.");

  LogInfo(*m_goomLogger, "Slot producer consumer thread stopping.");
  m_slotProducerConsumer.Stop();
  m_slotProducerConsumerThread.join();

  m_glScene.DestroyScene();

  LogProducerConsumerSummary();
}

auto GoomVisualization::LogProducerConsumerSummary() -> void
{
  LogInfo(*m_goomLogger, "Number of items produced: {}.", m_numItemsProduced);
  LogInfo(*m_goomLogger,
          "Average produce item time = {:.1f}ms.",
          m_totalProductionTimeInMs / static_cast<double>(m_numItemsProduced));
  LogInfo(*m_goomLogger, "Number of dropped audio samples: {}.", m_numberOfDroppedAudioSamples);
}

auto GoomVisualization::InitAudioValues(int32_t numChannels) noexcept -> void
{
  m_numChannels    = static_cast<size_t>(numChannels);
  m_audioSampleLen = m_numChannels * AudioSamples::AUDIO_SAMPLE_LEN;
}

auto GoomVisualization::InitSceneFrameData() noexcept -> void
{
  m_glScene.InitScene();
}

auto GoomVisualization::InitGoomControl() noexcept -> void
{
  m_goomControl->SetFrameData(m_glScene.GetFrameData(0));
}

auto GoomVisualization::AddAudioSample(const std::vector<float>& audioSample) -> void
{
  Expects(m_started);
  Expects(m_audioSampleLen == audioSample.size());

#if DEBUG_LOGGING
  // LogInfo(*m_goomLogger, "Moving audio sample to producer.");
#endif
  if (not m_slotProducerConsumer.AddResource(AudioSamples{m_numChannels, audioSample}))
  {
    ++m_numberOfDroppedAudioSamples;
#if DEBUG_LOGGING
    // LogWarn(*m_goomLogger, "### Resource queue full - skipping this audio sample.");
#endif
    return;
  }

  ++m_numAudioSamples;
}

auto GoomVisualization::UpdateTrack(const TrackInfo& track) -> void
{
  const auto artist          = not track.artist.empty() ? track.artist : track.albumArtist;
  const auto currentSongName = artist.empty() ? track.title : ((artist + " - ") + track.title);

  LogInfo(*m_goomLogger, "Current Title = '{}'", currentSongName);
  LogInfo(*m_goomLogger, "Genre = '{}', Duration = {}", track.genre, track.duration);

  m_goomControl->SetSongInfo({currentSongName, track.genre, track.duration});

#ifdef SAVE_AUDIO_BUFFERS
  m_audioBufferWriter = GetAudioBufferWriter(track.title);
#endif
}

auto GoomVisualization::ConsumeItem(const size_t slot) noexcept -> void
{
#if DEBUG_LOGGING
  LogInfo(*m_goomLogger, std_fmt::format("Consumer consuming slot {}.", slot));
#endif

  m_glScene.UpdateFrameData(slot);

#if DEBUG_LOGGING
  LogInfo(*m_goomLogger, std_fmt::format("Consumer consumed slot {}.", slot));
#endif
}

auto GoomVisualization::ProduceItem(const size_t slot, const AudioSamples& audioSamples) noexcept
    -> void
{
#if DEBUG_LOGGING
  LogInfo(*m_goomLogger, std_fmt::format("Producer producing slot {}.", slot));
#endif

  ++m_numItemsProduced;
  const auto startTime = std::chrono::system_clock::now();

  auto& frameData = m_glScene.GetFrameData(slot);

  m_goomControl->SetFrameData(frameData);
  m_goomControl->UpdateGoomBuffers(audioSamples);
  m_goomControl->UpdateFrameData(frameData);

  const auto duration = std::chrono::system_clock::now() - startTime;
  m_totalProductionTimeInMs +=
      static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(duration).count());

#if DEBUG_LOGGING
  LogInfo(*m_goomLogger, std_fmt::format("Producer produced slot {}.", slot));
#endif
}

} // namespace GOOM::VIS
