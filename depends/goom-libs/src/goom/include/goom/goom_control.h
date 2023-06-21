#pragma once

#include "frame_data.h"
#include "goom_types.h"
#include "spimpl.h"

#include <cstdint>
#include <memory>
#include <string>

namespace GOOM
{

class AudioSamples;
class GoomLogger;
class PixelBuffer;
struct GoomShaderVariables;

class GoomControl
{
public:
  [[nodiscard]] static auto MakeGoomLogger() noexcept -> std::unique_ptr<GoomLogger>;

  GoomControl() = delete;
  GoomControl(const Dimensions& dimensions,
              const std::string& resourcesDirectory,
              GoomLogger& goomLogger);

  enum class ShowMusicTitleType
  {
    AT_START,
    ALWAYS,
    NEVER,
  };
  auto SetShowMusicTitle(ShowMusicTitleType value) -> void;

  auto Start() -> void;
  auto Finish() -> void;

  struct SongInfo
  {
    std::string title{};
    std::string genre{};
    uint32_t duration{};
  };
  auto SetSongInfo(const SongInfo& songInfo) -> void;

  [[maybe_unused]] auto SetNoZooms(bool value) -> void;
  auto SetShowGoomState(bool value) -> void;
  auto SetDumpDirectory(const std::string& dumpDirectory) -> void;

  auto InitFrameData(std::vector<FrameData>& frameDataArray) noexcept -> void;
  auto SetFrameData(FrameData& frameData) -> void;
  auto UpdateGoomBuffers(const AudioSamples& audioSamples, const std::string& message = "") -> void;

  [[nodiscard]] auto GetLastShaderVariables() const -> const GoomShaderVariables&;

private:
  class GoomControlImpl;
  spimpl::unique_impl_ptr<GoomControlImpl> m_pimpl;
  friend class GoomControlLogger;
};

} // namespace GOOM
