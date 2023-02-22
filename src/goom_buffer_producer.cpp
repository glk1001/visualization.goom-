#undef NO_LOGGING

#include "goom_buffer_producer.h"

#include "goom/compiler_versions.h"
#include "goom/goom_config.h"
#include "goom/goom_control.h"
#include "goom/goom_graphic.h"
#include "goom/goom_logger.h"
#include "goom/goom_utils.h"

#include <random>
#include <string>

#ifdef SAVE_AUDIO_BUFFERS
#include "src/goom/src/utils/strutils.h"

#include <filesystem>
#endif

namespace GOOM
{

GoomBufferProducer::GoomBufferProducer(const TextureBufferDimensions& textureBufferDimensions,
                                       const std::string& resourcesDirectory,
                                       const GoomControl::ShowTitleType showTitle,
#ifdef SAVE_AUDIO_BUFFERS
                                       const std::string& audioBuffersSaveDir,
#endif
                                       GoomLogger& goomLogger) noexcept
  : m_textureBufferDimensions{textureBufferDimensions},
    m_goomLogger{&goomLogger},
    m_goomControl{std::make_unique<GoomControl>(
        Dimensions{m_textureBufferDimensions.width, m_textureBufferDimensions.height},
        resourcesDirectory,
        *m_goomLogger)}
#ifdef SAVE_AUDIO_BUFFERS
    ,
    m_audioBuffersSaveDir{audioBuffersSaveDir}
#endif
{
  LogDebug(*m_goomLogger, "Constructed producer.");
  m_goomControl->SetShowTitle(showTitle);
  Ensures(m_goomControl);
}

auto GoomBufferProducer::Start() -> void
{
  LogInfo(*m_goomLogger, "Starting goom buffer producer.");

  Expects(m_goomControl);
  Expects(not m_started);
  USED_FOR_DEBUGGING(m_started);
  m_started = true;

  LogInfo(*m_goomLogger, "Goom: Version: {}.", GetGoomLibVersionInfo());
  LogInfo(*m_goomLogger, "Goom: Compiler: {}.", GetCompilerVersion());
  LogInfo(*m_goomLogger, "Goom Library: Compiler: {}.", GetGoomLibCompilerVersion());
  LogInfo(*m_goomLogger, "Goom Library: Build Time: {}.", GetGoomLibBuildTime());

  LogInfo(*m_goomLogger,
          "Texture width, height = {}, {}.",
          m_textureBufferDimensions.width,
          m_textureBufferDimensions.height);

  // Goom will use same random sequence each time if the following is uncommented:
  //GoomControl::SetRandSeed(1);
  SetRandSeed(std::random_device{}());

  StartActivePixelBufferDataQueue();

  m_goomControl->Start();

  LogDebug(*m_goomLogger, "Started goom buffer producer.");
}

auto GoomBufferProducer::Stop() -> void
{
  LogDebug(*m_goomLogger, "Stopping goom buffer producer.");

  Expects(m_started);

  m_goomControl->Finish();

  StopProcessGoomBuffersThread();

  m_audioBufferNum = 0;

  LogInfo(*m_goomLogger, "Stopped goom buffer producer.");
}

auto GoomBufferProducer::SetNumChannels(const uint32_t value) -> void
{
  m_numChannels    = static_cast<size_t>(value);
  m_audioBufferLen = m_numChannels * AudioSamples::AUDIO_SAMPLE_LEN;
  m_audioBufferNum = 0;
}

auto GoomBufferProducer::StartProcessGoomBuffersThread() -> void
{
  LogInfo(*m_goomLogger, "Starting process Goom buffers thread.");
  m_processBuffersThread = std::thread{&GoomBufferProducer::ProcessGoomBuffersThread, this};
}

auto GoomBufferProducer::StopProcessGoomBuffersThread() -> void
{
  LogInfo(*m_goomLogger, "Stopping process Goom buffers thread.");

  auto lock                          = std::unique_lock{m_mutex};
  m_processGoomBuffersThreadFinished = true;
  m_wait.notify_one();
  lock.unlock();

  if (m_processBuffersThread.joinable())
  {
    m_processBuffersThread.join();
  }

  LogInfo(*m_goomLogger, "Process Goom buffers thread stopped.");
}

auto GoomBufferProducer::ProcessAudioData(const float* const audioData,
                                          const size_t audioDataLength) -> void
{
  Expects(m_started);

  ++m_audioBufferNum;

  if (1 == m_audioBufferNum)
  {
    LogInfo(*m_goomLogger, "Starting audio data processing.");
    StartProcessGoomBuffersThread();
  }

  const auto lock = std::scoped_lock{m_mutex};
  if (m_audioBuffer.DataAvailable() >= CIRCULAR_BUFFER_SIZE)
  {
    return;
  }

  m_audioBuffer.Write(audioData, static_cast<size_t>(audioDataLength));
  m_wait.notify_one();
}

auto GoomBufferProducer::UpdateTrack(const GoomControl::SongInfo& track) -> void
{
  Expects(m_started);

  LogInfo(*m_goomLogger, "Current Title = '{}'", track.title);
  LogInfo(*m_goomLogger, "Genre = '{}', Duration = {}", track.genre, track.duration);

  m_goomControl->SetSongInfo(track);

#ifdef SAVE_AUDIO_BUFFERS
  m_audioBufferWriter = GetAudioBufferWriter(track.title);
#endif
}

auto GoomBufferProducer::ProcessGoomBuffersThread() -> void
{
  auto floatAudioData = std::vector<float>(m_audioBufferLen);

  while (true)
  {
    auto lock = std::unique_lock{m_mutex};
    if (m_processGoomBuffersThreadFinished)
    {
      break;
    }

    if (m_audioBuffer.DataAvailable() < m_audioBufferLen)
    {
      m_wait.wait(lock);
    }
    if (const auto read = m_audioBuffer.Read(floatAudioData.data(), m_audioBufferLen);
        read != m_audioBufferLen)
    {
      LogWarn(*m_goomLogger,
              "Num read audio length {} != {} = expected audio data length - "
              "skipping this.",
              read,
              m_audioBufferLen);
      continue;
    }
    lock.unlock();

    if (m_processGoomBuffersThreadFinished)
    {
      break;
    }

    lock.lock();
    if (m_activePixelBufferDataQueue.size() > MAX_ACTIVE_QUEUE_LENGTH)
    {
      // Too far behind, skip this audio data.
      continue;
    }
    lock.unlock();

    auto pixelBufferData = PixelBufferData{};
    lock.lock();
    if (m_storedPixelBufferDataQueue.empty())
    {
      pixelBufferData = MakePixelBufferData();
    }
    else
    {
      pixelBufferData = m_storedPixelBufferDataQueue.front();
      m_storedPixelBufferDataQueue.pop();
    }
    lock.unlock();

    UpdateGoomBuffer(floatAudioData, pixelBufferData);

    lock.lock();
    m_activePixelBufferDataQueue.push(pixelBufferData);
    lock.unlock();
  }
}

inline auto GoomBufferProducer::UpdateGoomBuffer(const std::vector<float>& floatAudioData,
                                                 PixelBufferData& pixelBufferData) -> void
{
  const auto audioData = AudioSamples{m_numChannels, floatAudioData};
  m_goomControl->SetGoomBuffer(pixelBufferData.pixelBuffer);
  m_goomControl->UpdateGoomBuffer(audioData);
  pixelBufferData.goomShaderVariables = m_goomControl->GetLastShaderVariables();

#ifdef SAVE_AUDIO_BUFFERS
  SaveAudioBuffer(floatAudioData);
#endif
}

auto GoomBufferProducer::StartActivePixelBufferDataQueue() -> void
{
  // Make one initial frame in black
  const auto initialBufferData = PixelBufferData{MakePixelBufferData()};
  initialBufferData.pixelBuffer->Fill(GOOM::BLACK_PIXEL);
  m_activePixelBufferDataQueue.push(initialBufferData);
}

auto GoomBufferProducer::GetNextActivePixelBufferData() -> PixelBufferData
{
  auto pixelBufferData = PixelBufferData{};

  const auto lock = std::scoped_lock{m_mutex};
  if (not m_activePixelBufferDataQueue.empty())
  {
    pixelBufferData = m_activePixelBufferDataQueue.front();
    m_activePixelBufferDataQueue.pop();
  }

  return pixelBufferData;
}

inline auto GoomBufferProducer::MakePixelBufferData() const -> PixelBufferData
{
  auto pixelBufferData        = PixelBufferData{};
  pixelBufferData.pixelBuffer = std::make_shared<PixelBuffer>(
      Dimensions{m_textureBufferDimensions.width, m_textureBufferDimensions.height});
  return pixelBufferData;
}

auto GoomBufferProducer::PushUsedPixels(const PixelBufferData& pixelBufferData) -> void
{
  const auto lock = std::scoped_lock{m_mutex};
  m_storedPixelBufferDataQueue.push(pixelBufferData);
}

#ifdef SAVE_AUDIO_BUFFERS
namespace
{

[[nodiscard]] auto ReplaceIllegalFilenameChars(const std::string& str) noexcept -> std::string
{
  using GOOM::UTILS::FindAndReplaceAll;

  auto legalStr = str;

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

} // namespace

auto GoomBufferProducer::GetAudioBufferWriter(const std::string& songName) const
    -> std::unique_ptr<AudioBufferWriter>
{
  using GOOM::PATH_SEP;

  const auto cleanedSongName   = ReplaceIllegalFilenameChars(songName);
  const auto songSaveDirectory = m_audioBuffersSaveDir + PATH_SEP + cleanedSongName;

  if (std::filesystem::exists(songSaveDirectory))
  {
    std::filesystem::remove_all(songSaveDirectory);
  }
  if (not std::filesystem::create_directories(songSaveDirectory))
  {
    throw std::runtime_error{std20::format("Could not create directory '{}'.", songSaveDirectory)};
  }

  static constexpr auto* AUDIO_BUFFERS_FILE_PREFIX = "audio";
  const auto saveFilePrefix = songSaveDirectory + PATH_SEP + AUDIO_BUFFERS_FILE_PREFIX;

  return std::make_unique<AudioBufferWriter>(saveFilePrefix);
}

auto GoomBufferProducer::SaveAudioBuffer(const std::vector<float>& floatAudioData) -> void
{
  if (m_audioBufferWriter == nullptr)
  {
    return;
  }

  using GOOM::UTILS::BufferView;
  const auto audioBuffer = BufferView<float>{floatAudioData.size(), floatAudioData.data()};
  const auto bufferNum   = m_audioBufferWriter->GetCurrentBufferNum();
  /**
  if (bufferNum == 0) {
    for (size_t i = 0; i < floatAudioData.size(); ++i)
    {
      LogInfo("floatAudioData[{}] = {}", i, floatAudioData[i]);
    }
  }
   **/

  const auto currentFilename = m_audioBufferWriter->GetCurrentFilename();
  m_audioBufferWriter->Write(audioBuffer, true);
  LogInfo(*m_goomLogger, "Wrote audio data buffer {} to file '{}'.", bufferNum, currentFilename);
}

#endif

} // namespace GOOM
