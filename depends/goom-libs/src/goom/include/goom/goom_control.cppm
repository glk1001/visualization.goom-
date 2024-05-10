module;

#include "goom_logger.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>

export module Goom.Lib.GoomControl;

import Goom.Lib.FrameData;
import Goom.Lib.GoomGraphic;
import Goom.Lib.GoomTypes;
import Goom.Lib.SoundInfo;
import Goom.Lib.SPimpl;

export namespace GOOM
{

class GoomControl
{
public:
  [[nodiscard]] static auto MakeGoomLogger() noexcept -> std::unique_ptr<GoomLogger>;

  GoomControl() = delete;
  GoomControl(const Dimensions& dimensions,
              const std::string& resourcesDirectory,
              GoomLogger& goomLogger);

  auto SetShowSongTitle(ShowSongTitleType value) -> void;

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

  auto SetFrameData(FrameData& frameData) -> void;
  auto UpdateGoomBuffers(const AudioSamples& audioSamples, const std::string& message = "") -> void;

  [[nodiscard]] auto GetFrameData() const noexcept -> const FrameData&;
  [[nodiscard]] auto GetNumPoolThreads() const noexcept -> size_t;

private:
  class GoomControlImpl;
  spimpl::unique_impl_ptr<GoomControlImpl> m_pimpl;
  friend class GoomControlLogger;
};

} // namespace GOOM
