#pragma once

#include "goom_all_visual_fx.h"
#include "goom_music_settings_reactor.h"
#include "visual_fx/filters/filter_settings_service.h"

#include <chrono>
#include <cstdint>
#include <memory>
#include <string_view>

namespace GOOM::CONTROL
{

class GoomStateDump
{
public:
  GoomStateDump(const PluginInfo& goomInfo,
                const GoomAllVisualFx& visualFx,
                const GoomMusicSettingsReactor& musicSettingsReactor,
                const VISUAL_FX::FILTERS::FilterSettingsService& filterSettingsService) noexcept;
  GoomStateDump(const GoomStateDump&) noexcept = delete;
  GoomStateDump(GoomStateDump&&) noexcept = delete;
  ~GoomStateDump() noexcept;
  auto operator=(const GoomStateDump&) -> GoomStateDump& = delete;
  auto operator=(GoomStateDump&&) -> GoomStateDump& = delete;

  void SetSongTitle(const std::string& songTitle);
  void SetGoomSeed(uint64_t goomSeed);

  void Start();

  void AddCurrentState();
  void DumpData(const std::string& directory);

private:
  static constexpr uint32_t MIN_TIMELINE_ELEMENTS_TO_DUMP = 10;

  const PluginInfo& m_goomInfo;
  const GoomAllVisualFx& m_visualFx;
  //  const GoomMusicSettingsReactor& m_musicSettingsReactor;
  const VISUAL_FX::FILTERS::FilterSettingsService& m_filterSettingsService;

  using Ms = std::chrono::milliseconds;
  std::chrono::high_resolution_clock::time_point m_prevTimeHiRes{};

  class CumulativeState;
  std::unique_ptr<CumulativeState> m_cumulativeState;

  std::string m_songTitle{};
  std::string m_dateTime{};
  uint64_t m_goomSeed{};

  std::string m_datedDirectory{};
  void SetCurrentDatedDirectory(const std::string& parentDirectory);
  void DumpSummary() const;
  template<typename T>
  void DumpDataArray(const std::string& filename, const std::vector<T>& dataArray) const;
};

inline void GoomStateDump::SetSongTitle(const std::string& songTitle)
{
  m_songTitle = songTitle;
}

inline void GoomStateDump::SetGoomSeed(uint64_t goomSeed)
{
  m_goomSeed = goomSeed;
}

} // namespace GOOM::CONTROL
