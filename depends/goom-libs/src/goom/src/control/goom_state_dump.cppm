module;

#include "goom/goom_config.h"

#include <chrono>
#include <cstdint>
#include <string>
#include <vector>

// #define DO_GOOM_STATE_DUMP

export module Goom.Control.GoomStateDump;

namespace GOOM
{
class GoomControl;
class GoomLogger;
}

#ifdef DO_GOOM_STATE_DUMP

import Goom.Control.GoomMusicSettingsReactor;
import Goom.Control.GoomStateHandler;
import Goom.FilterFx.FilterSettingsService;
import Goom.Utils.Stopwatch;
import Goom.Lib.SoundInfo;

export namespace GOOM::CONTROL
{

class GoomStateDump
{
public:
  GoomStateDump(const PluginInfo& goomInfo,
                GoomLogger& goomLogger,
                const GoomControl& goomControl,
                const GoomAllVisualFx& visualFx,
                const GoomMusicSettingsReactor& musicSettingsReactor,
                const FILTER_FX::FilterSettingsService& filterSettingsService) noexcept;

  auto SetSongTitle(const std::string& songTitle) noexcept -> void;
  auto SetGoomSeed(uint64_t goomSeed) noexcept -> void;
  auto SetStopWatch(const UTILS::Stopwatch& stopwatch) noexcept -> void;

  auto Start() noexcept -> void;

  auto AddCurrentState() noexcept -> void;
  auto DumpData(const std::string& directory) -> void;

private:
  static constexpr uint32_t MIN_TIMELINE_ELEMENTS_TO_DUMP = 10;

  const PluginInfo* m_goomInfo;
  GoomLogger* m_goomLogger;
  const GoomControl* m_goomControl;
  const GoomAllVisualFx* m_visualFx;
  const FILTER_FX::FilterSettingsService* m_filterSettingsService;

  using Ms = std::chrono::milliseconds;
  std::chrono::high_resolution_clock::time_point m_prevTimeHiRes{};

  class CumulativeState;
  spimpl::unique_impl_ptr<CumulativeState> m_cumulativeState;

  std::string m_songTitle{};
  std::string m_dateTime{};
  uint64_t m_goomSeed{};
  const UTILS::Stopwatch* m_stopwatch{};

  std::string m_datedDirectory{};
  auto SetCurrentDatedDirectory(const std::string& parentDirectory) -> void;
  auto DumpSummary() const -> void;
  template<typename T>
  auto DumpDataArray(const std::string& filename, const std::vector<T>& dataArray) const -> void;
  template<typename T>
  [[nodiscard]] static auto GetFormattedRowStr(uint64_t value1, T value2) -> std::string;
};

} // namespace GOOM::CONTROL

namespace GOOM::CONTROL
{

inline auto GoomStateDump::SetSongTitle(const std::string& songTitle) noexcept -> void
{
  m_songTitle = songTitle;
}

inline auto GoomStateDump::SetGoomSeed(const uint64_t goomSeed) noexcept -> void
{
  m_goomSeed = goomSeed;
}

inline auto GoomStateDump::SetStopWatch(const UTILS::Stopwatch& stopwatch) noexcept -> void
{
  m_stopwatch = &stopwatch;
}

} // namespace GOOM::CONTROL

#endif // DO_GOOM_STATE_DUMP
