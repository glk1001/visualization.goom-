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
                                       const GoomControl::ShowMusicTitleType showMusicTitle,
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
  m_goomControl->SetShowMusicTitle(showMusicTitle);
  Ensures(nullptr != m_goomControl);
}

auto GoomBufferProducer::Start() -> void
{
  LogInfo(*m_goomLogger, "Starting goom buffer producer.");

  Expects(m_goomControl != nullptr);
  Expects(not m_running);
  m_running = true;

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

  Expects(m_running);

  m_goomControl->Finish();

  m_running = false;
  m_audioBuffer_cv.notify_one();

  LogInfo(*m_goomLogger, "Stopped goom buffer producer.");
}

auto GoomBufferProducer::SetNumChannels(const uint32_t value) -> void
{
  m_numChannels    = static_cast<size_t>(value);
  m_audioBufferLen = m_numChannels * AudioSamples::AUDIO_SAMPLE_LEN;
  m_rawAudioData.resize(m_audioBufferLen);
  m_audioBufferNum = 0;
}

auto GoomBufferProducer::ProcessAudioData(const std_spn::span<const float>& audioData) -> void
{
  Expects(m_running);

  ++m_audioBufferNum;

  const auto lock = std::scoped_lock<std::mutex>{m_mutex};
  if (m_audioBuffer.FreeSpace() < audioData.size())
  {
    // TODO - is this a good idea?
    // Lose the audio?????
    return;
  }

  m_audioBuffer.Write(audioData);
  m_audioBuffer_cv.notify_one();
}

auto GoomBufferProducer::UpdateTrack(const GoomControl::SongInfo& track) -> void
{
  Expects(m_running);

  LogInfo(*m_goomLogger, "Current Title = '{}'", track.title);
  LogInfo(*m_goomLogger, "Genre = '{}', Duration = {}", track.genre, track.duration);

  m_goomControl->SetSongInfo(track);

#ifdef SAVE_AUDIO_BUFFERS
  m_audioBufferWriter = GetAudioBufferWriter(track.title);
#endif
}

auto GoomBufferProducer::ProcessGoomBuffersThread() -> void
{
  while (m_running)
  {
    ProduceNextBuffer();
  }
}

auto GoomBufferProducer::ProduceNextBuffer() -> void
{
  auto lock = std::unique_lock<std::mutex>{m_mutex};
  if (not m_running)
  {
    return;
  }

  if (m_audioBuffer.DataAvailable() < m_audioBufferLen)
  {
    m_audioBuffer_cv.wait(
        lock,
        [this] { return (not m_running) or (m_audioBuffer.DataAvailable() >= m_audioBufferLen); });
  }
  if (not m_running)
  {
    return;
  }

  m_audioBuffer.Read(m_rawAudioData);
  if (not m_running)
  {
    return;
  }

  if (m_activePixelBufferDataQueue.size() > MAX_ACTIVE_QUEUE_LENGTH)
  {
    // Too far behind, skip this audio data.
    return;
  }

  auto pixelBufferData = PixelBufferData{};
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

  UpdateGoomBuffer(pixelBufferData);

  lock.lock();
  m_activePixelBufferDataQueue.push(pixelBufferData);
}

inline auto GoomBufferProducer::UpdateGoomBuffer(PixelBufferData& pixelBufferData) -> void
{
  const auto audioData = AudioSamples{m_numChannels, m_rawAudioData};
  m_goomControl->SetGoomBuffer(pixelBufferData.pixelBuffer);
  m_goomControl->UpdateGoomBuffers(audioData);
  pixelBufferData.goomShaderVariables = m_goomControl->GetLastShaderVariables();

#ifdef SAVE_AUDIO_BUFFERS
  SaveAudioBuffer(rawAudioData);
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

  const auto lock = std::scoped_lock<std::mutex>{m_mutex};
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
  pixelBufferData.pixelBuffer = std::make_shared<PixelBufferVector>(
      Dimensions{m_textureBufferDimensions.width, m_textureBufferDimensions.height});
  return pixelBufferData;
}

auto GoomBufferProducer::PushUsedPixels(const PixelBufferData& pixelBufferData) -> void
{
  const auto lock = std::scoped_lock<std::mutex>{m_mutex};
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
    throw std::runtime_error{
        STD20_2::format("Could not create directory '{}'.", songSaveDirectory)};
  }

  static constexpr auto* AUDIO_BUFFERS_FILE_PREFIX = "audio";
  const auto saveFilePrefix = songSaveDirectory + PATH_SEP + AUDIO_BUFFERS_FILE_PREFIX;

  return std::make_unique<AudioBufferWriter>(saveFilePrefix);
}

auto GoomBufferProducer::SaveAudioBuffer(const std::vector<float>& rawAudioData) -> void
{
  if (m_audioBufferWriter == nullptr)
  {
    return;
  }

  using GOOM::UTILS::BufferView;
  const auto audioBuffer = BufferView<float>{rawAudioData.size(), rawAudioData.data()};
  const auto bufferNum   = m_audioBufferWriter->GetCurrentBufferNum();
  /**
  if (bufferNum == 0) {
    for (size_t i = 0; i < rawAudioData.size(); ++i)
    {
      LogInfo("rawAudioData[{}] = {}", i, rawAudioData[i]);
    }
  }
   **/

  const auto currentFilename = m_audioBufferWriter->GetCurrentFilename();
  m_audioBufferWriter->Write(audioBuffer, true);
  LogInfo(*m_goomLogger, "Wrote audio data buffer {} to file '{}'.", bufferNum, currentFilename);
}

#endif

} // namespace GOOM
