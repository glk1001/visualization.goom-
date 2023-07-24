#pragma once

#include "displacement_filter.h"
#include "gl_render_types.h"
#include "goom/sound_info.h"
#include "slot_producer_consumer.h"

#include <cstdint>
#include <memory>
#include <string>
#include <thread>

namespace GOOM
{
class GoomLogger;
class GoomControl;
}

namespace GOOM::VIS
{

class GoomVisualization
{
public:
  GoomVisualization(GOOM::GoomLogger& goomLogger,
                    const std::string& resourcesDir,
                    uint32_t consumeWaitForProducerMs,
                    const std::string& shaderDir,
                    const TextureBufferDimensions& textureBufferDimensions);
  GoomVisualization(GOOM::GoomLogger& goomLogger,
                    const std::string& resourcesDir,
                    uint32_t consumeWaitForProducerMs,
                    const TextureBufferDimensions& textureBufferDimensions,
                    std::unique_ptr<GOOM::OPENGL::DisplacementFilter>&& glScene);
  GoomVisualization(const GoomVisualization&) noexcept = delete;
  GoomVisualization(GoomVisualization&&) noexcept      = delete;
  ~GoomVisualization() noexcept;
  auto operator=(const GoomVisualization&) -> GoomVisualization& = delete;
  auto operator=(GoomVisualization&&) -> GoomVisualization&      = delete;

  static auto SetRandomSeed(uint64_t seed) noexcept -> void;
  auto SetWindowDimensions(const WindowDimensions& windowDimensions) noexcept -> void;
  auto SetShowSongTitle(ShowSongTitleType showMusicTitleType) -> void;
  auto SetShowGoomState(bool value) -> void;
  auto SetDumpDirectory(const std::string& dumpDirectory) -> void;

  auto Start(int numChannels) -> void;
  auto StartThread() -> void;
  auto Stop() -> void;

  auto AddAudioSample(const std::vector<float>& audioSample) -> bool;

  struct TrackInfo
  {
    std::string title{};
    std::string artist{};
    std::string albumArtist{};
    std::string genre{};
    uint32_t duration = 0U;
  };
  auto UpdateTrack(const TrackInfo& track) -> void;

  [[nodiscard]] auto GetScene() noexcept -> GOOM::OPENGL::DisplacementFilter&;
  [[nodiscard]] auto GetNumAudioSamples() const noexcept -> uint32_t;

private:
  bool m_started = false;
  GOOM::GoomLogger* m_goomLogger;
  uint32_t m_consumeWaitForProducerMs;
  auto InitConstructor() noexcept -> void;

  std::unique_ptr<GOOM::OPENGL::DisplacementFilter> m_glScene;
  auto InitSceneFrameData() noexcept -> void;

  std::unique_ptr<GOOM::GoomControl> m_goomControl;
  auto InitGoomControl() noexcept -> void;

  GOOM::SlotProducerConsumer<GOOM::AudioSamples> m_slotProducerConsumer;
  GOOM::SlotProducerIsDriving<GOOM::AudioSamples> m_slotProducerIsDriving;
  std::thread m_slotProducerConsumerThread{};
  auto ProduceItem(size_t slot, const GOOM::AudioSamples& audioSamples) noexcept -> void;
  auto ConsumeItem(size_t slot) noexcept -> void;
  uint32_t m_numberOfDroppedAudioSamples = 0U;
  auto LogProducerConsumerSummary() -> void;
  double m_totalProductionTimeInMs = 0.0;
  uint64_t m_numItemsProduced      = 0U;

  size_t m_numChannels       = 0;
  size_t m_audioSampleLen    = 0;
  uint32_t m_numAudioSamples = 0U;
  auto InitAudioValues(int32_t numChannels) noexcept -> void;
};

inline auto GoomVisualization::GetScene() noexcept -> GOOM::OPENGL::DisplacementFilter&
{
  return *m_glScene;
}

inline auto GoomVisualization::GetNumAudioSamples() const noexcept -> uint32_t
{
  return m_numAudioSamples;
}

} // namespace GOOM::VIS
