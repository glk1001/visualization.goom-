#include "goom_state_dump.h"

#undef NO_LOGGING

#include "goom/logging.h"
#include "goom_all_visual_fx.h"
#include "goom_graphic.h"
#include "goom_music_settings_reactor.h"
#include "visual_fx/filters/filter_settings_service.h"

#include <filesystem>
#include <fstream>
#include <string>

namespace GOOM::CONTROL
{

using UTILS::Logging;
using VISUAL_FX::FILTERS::HypercosOverlay;
using VISUAL_FX::FILTERS::ZoomFilterColorSettings;
using VISUAL_FX::FILTERS::ZoomFilterEffectsSettings;
using VISUAL_FX::FILTERS::ZoomFilterMode;
using VISUAL_FX::FILTERS::ZoomFilterSettings;

class GoomStateDump::CumulativeState
{
public:
  CumulativeState() noexcept;

  void Reset();
  void IncrementUpdateNum();

  [[nodiscard]] auto GetNumUpdates() const -> uint32_t;

  void AddCurrentUpdateTime(uint32_t timeInMs);

  void AddCurrentGoomState(GoomStates goomState);
  void AddCurrentFilterMode(ZoomFilterMode filterMode);
  void AddCurrentHypercosOverlay(HypercosOverlay hypercosOverlay);
  void AddCurrentBlockyWavyEffect(bool value);
  void AddCurrentImageVelocityEffect(bool value);
  void AddCurrentNoiseEffect(bool value);
  void AddCurrentPlaneEffect(bool value);
  void AddCurrentRotationEffect(bool value);
  void AddCurrentTanEffect(bool value);

  void AddCurrentTimeSinceLastGoom(uint32_t value);
  void AddCurrentTimeSinceLastBigGoom(uint32_t value);
  void AddCurrentTotalGoomsInCurrentCycle(uint32_t value);
  void AddCurrentGoomPower(float value);
  void AddCurrentGoomVolume(float value);

  [[nodiscard]] auto GetUpdateTimesInMs() const -> const std::vector<uint32_t>&;

  [[nodiscard]] auto GetGoomStates() const -> const std::vector<uint8_t>&;
  [[nodiscard]] auto GetFilterModes() const -> const std::vector<uint8_t>&;
  [[nodiscard]] auto GetHypercosOverlays() const -> const std::vector<uint8_t>&;
  [[nodiscard]] auto GetBlockyWavyEffects() const -> const std::vector<uint8_t>&;
  [[nodiscard]] auto GetImageVelocityEffects() const -> const std::vector<uint8_t>&;
  [[nodiscard]] auto GetNoiseEffects() const -> const std::vector<uint8_t>&;
  [[nodiscard]] auto GetPlaneEffects() const -> const std::vector<uint8_t>&;
  [[nodiscard]] auto GetRotationEffects() const -> const std::vector<uint8_t>&;
  [[nodiscard]] auto GetTanEffects() const -> const std::vector<uint8_t>&;

  [[nodiscard]] auto GetTimeSinceLastGoom() const -> const std::vector<uint32_t>&;
  [[nodiscard]] auto GetTimeSinceLastBigGoom() const -> const std::vector<uint32_t>&;
  [[nodiscard]] auto GetTotalGoomsInCurrentCycle() const -> const std::vector<uint32_t>&;
  [[nodiscard]] auto GetGoomPower() const -> const std::vector<float>&;
  [[nodiscard]] auto GetGoomVolume() const -> const std::vector<float>&;

private:
  static constexpr uint32_t INITIAL_NUM_UPDATES_ESTIMATE = 5 * 60 * 60 * 25;
  static constexpr uint32_t EXTRA_NUM_UPDATES_ESTIMATE = 1 * 60 * 60 * 25;
  uint32_t m_numUpdatesEstimate = INITIAL_NUM_UPDATES_ESTIMATE;
  uint32_t m_updateNum = 0;

  std::vector<uint32_t> m_updateTimesInMs{};
  std::vector<uint8_t> m_goomStates{};
  std::vector<uint8_t> m_filterModes{};
  std::vector<uint8_t> m_hypercosOverlays{};
  std::vector<uint8_t> m_blockyWavyEffects{};
  std::vector<uint8_t> m_imageVelocityEffects{};
  std::vector<uint8_t> m_noiseEffects{};
  std::vector<uint8_t> m_planeEffects{};
  std::vector<uint8_t> m_rotationEffects{};
  std::vector<uint8_t> m_tanEffects{};

  std::vector<uint32_t> m_timeSinceLastGoom{};
  std::vector<uint32_t> m_timeSinceLastBigGoom{};
  std::vector<uint32_t> m_totalGoomsInCurrentCycle{};
  std::vector<float> m_goomPower{};
  std::vector<float> m_goomVolume{};

  void Reserve();
};

GoomStateDump::GoomStateDump(
    const PluginInfo& goomInfo,
    const GoomAllVisualFx& visualFx,
    [[maybe_unused]] const GoomMusicSettingsReactor& musicSettingsReactor,
    const VISUAL_FX::FILTERS::FilterSettingsService& filterSettingsService) noexcept
  : m_goomInfo{goomInfo},
    m_visualFx{visualFx},
    //    m_musicSettingsReactor{musicSettingsReactor},
    m_filterSettingsService{filterSettingsService},
    m_cumulativeState{std::make_unique<CumulativeState>()}
{
}

GoomStateDump::~GoomStateDump() noexcept = default;

void GoomStateDump::Start()
{
  m_cumulativeState->Reset();

  m_prevTimeHiRes = std::chrono::high_resolution_clock::now();
}

void GoomStateDump::AddCurrentState()
{
  const auto timeNow = std::chrono::high_resolution_clock::now();
  const Ms diff = std::chrono::duration_cast<Ms>(timeNow - m_prevTimeHiRes);
  const auto timeOfUpdateInMs = static_cast<uint32_t>(diff.count());
  m_cumulativeState->AddCurrentUpdateTime(timeOfUpdateInMs);
  m_prevTimeHiRes = timeNow;

  m_cumulativeState->AddCurrentGoomState(m_visualFx.GetCurrentState());
  m_cumulativeState->AddCurrentFilterMode(m_filterSettingsService.GetCurrentFilterMode());

  const ZoomFilterSettings filterSettings = m_filterSettingsService.GetFilterSettings();

  const ZoomFilterColorSettings filterColorSettings = filterSettings.filterColorSettings;
  m_cumulativeState->AddCurrentBlockyWavyEffect(filterColorSettings.blockyWavy);

  const ZoomFilterEffectsSettings filterEffectsSettings = filterSettings.filterEffectsSettings;
  m_cumulativeState->AddCurrentHypercosOverlay(filterEffectsSettings.hypercosOverlay);
  m_cumulativeState->AddCurrentImageVelocityEffect(filterEffectsSettings.imageVelocityEffect);
  m_cumulativeState->AddCurrentNoiseEffect(filterEffectsSettings.noiseEffect);
  m_cumulativeState->AddCurrentPlaneEffect(filterEffectsSettings.planeEffect);
  m_cumulativeState->AddCurrentRotationEffect(filterEffectsSettings.rotationEffect);
  m_cumulativeState->AddCurrentTanEffect(filterEffectsSettings.tanEffect);

  const SoundInfo& soundInfo = m_goomInfo.GetSoundInfo();
  m_cumulativeState->AddCurrentTimeSinceLastGoom(soundInfo.GetTimeSinceLastGoom());
  m_cumulativeState->AddCurrentTimeSinceLastBigGoom(soundInfo.GetTimeSinceLastBigGoom());
  m_cumulativeState->AddCurrentTotalGoomsInCurrentCycle(soundInfo.GetTotalGoomsInCurrentCycle());
  m_cumulativeState->AddCurrentGoomPower(soundInfo.GetGoomPower());
  m_cumulativeState->AddCurrentGoomVolume(soundInfo.GetVolume());

  m_cumulativeState->IncrementUpdateNum();
}

[[nodiscard]] auto GetCurrentDateTimeAsStr() -> std::string
{
  const std::time_t t = std::time(nullptr);
  if (char str[100]; std::strftime(str, sizeof(str), "%Y-%m-%d_%H-%M-%S", std::localtime(&t)))
  {
    return std::string{str};
  }
  return "TIME_ERROR";
}

void GoomStateDump::DumpData(const std::string& directory)
{
  if (m_cumulativeState->GetNumUpdates() < MIN_TIMELINE_ELEMENTS_TO_DUMP)
  {
    LogWarn("Not dumping. Too few goom updates: {} < {}.", m_cumulativeState->GetNumUpdates(),
            MIN_TIMELINE_ELEMENTS_TO_DUMP);
    return;
  }

  m_dateTime = GetCurrentDateTimeAsStr();

  SetCurrentDatedDirectory(directory);

  DumpSummary();

  DumpDataArray("update_times", m_cumulativeState->GetUpdateTimesInMs());

  DumpDataArray("goom_states", m_cumulativeState->GetGoomStates());
  DumpDataArray("filter_modes", m_cumulativeState->GetFilterModes());
  DumpDataArray("hypercos_overlays", m_cumulativeState->GetHypercosOverlays());
  DumpDataArray("blocky_wavy_effects", m_cumulativeState->GetBlockyWavyEffects());
  DumpDataArray("image_velocity_effects", m_cumulativeState->GetImageVelocityEffects());
  DumpDataArray("noise_effects", m_cumulativeState->GetNoiseEffects());
  DumpDataArray("plane_effects", m_cumulativeState->GetPlaneEffects());
  DumpDataArray("rotation_effects", m_cumulativeState->GetRotationEffects());
  DumpDataArray("tan_effects", m_cumulativeState->GetTanEffects());

  DumpDataArray("time_since_last_goom", m_cumulativeState->GetTimeSinceLastGoom());
  DumpDataArray("time_since_last_big_goom", m_cumulativeState->GetTimeSinceLastBigGoom());
  DumpDataArray("total_gooms_in_current_cycle", m_cumulativeState->GetTotalGoomsInCurrentCycle());
  DumpDataArray("goom_power", m_cumulativeState->GetGoomPower());
  DumpDataArray("goom_volume", m_cumulativeState->GetGoomVolume());
}

void GoomStateDump::DumpSummary() const
{
  static constexpr const char* SUMMARY_FILENAME = "summary.dat";
  std::ofstream out{};
  out.open(m_datedDirectory + "/" + SUMMARY_FILENAME, std::ofstream::out);

  out << "Song:   " << m_songTitle << "\n";
  out << "Date:   " << m_dateTime << "\n";
  out << "Seed:   " << m_goomSeed << "\n";
  out << "Width:  " << m_goomInfo.GetScreenInfo().width << "\n";
  out << "Height: " << m_goomInfo.GetScreenInfo().height << "\n";

  out.close();
}

void GoomStateDump::SetCurrentDatedDirectory(const std::string& parentDirectory)
{
  m_datedDirectory = parentDirectory + "/" + m_dateTime;
  LogInfo("Dumping state data to \"{}\"...", m_datedDirectory);
  std::filesystem::create_directories(m_datedDirectory);
}

inline std::ostream& operator<<(std::ostream& out, const uint8_t value)
{
  return out << static_cast<uint32_t>(value);
}

template<typename T>
void GoomStateDump::DumpDataArray(const std::string& filename,
                                  const std::vector<T>& dataArray) const
{
  const uint32_t dataLen = m_cumulativeState->GetNumUpdates();
  LogInfo("Dumping Goom state data ({} values) to \"{}\".", dataLen, filename);

  static constexpr const char* EXT = ".dat";
  std::ofstream out{};
  out.open(m_datedDirectory + "/" + filename + EXT, std::ofstream::out);

  for (size_t i = 0; i < dataLen; ++i)
  {
    out << dataArray[i] << "\n";
  }

  out.close();
}

GoomStateDump::CumulativeState::CumulativeState() noexcept = default;

inline void GoomStateDump::CumulativeState::Reset()
{
  m_updateNum = 0;

  Reserve();
}

inline void GoomStateDump::CumulativeState::Reserve()
{
  m_updateTimesInMs.reserve(m_numUpdatesEstimate);
  m_goomStates.reserve(m_numUpdatesEstimate);
  m_filterModes.reserve(m_numUpdatesEstimate);
  m_hypercosOverlays.reserve(m_numUpdatesEstimate);
  m_blockyWavyEffects.reserve(m_numUpdatesEstimate);
  m_imageVelocityEffects.reserve(m_numUpdatesEstimate);
  m_noiseEffects.reserve(m_numUpdatesEstimate);
  m_planeEffects.reserve(m_numUpdatesEstimate);
  m_rotationEffects.reserve(m_numUpdatesEstimate);
  m_tanEffects.reserve(m_numUpdatesEstimate);
}

inline void GoomStateDump::CumulativeState::IncrementUpdateNum()
{
  ++m_updateNum;

  if (m_updateNum > m_numUpdatesEstimate)
  {
    m_numUpdatesEstimate += EXTRA_NUM_UPDATES_ESTIMATE;
    Reserve();
  }
}

inline void GoomStateDump::CumulativeState::AddCurrentUpdateTime(const uint32_t timeInMs)
{
  m_updateTimesInMs.push_back(timeInMs);
}

inline void GoomStateDump::CumulativeState::AddCurrentGoomState(const GoomStates goomState)
{
  m_goomStates.push_back(static_cast<uint8_t>(goomState));
}

inline void GoomStateDump::CumulativeState::AddCurrentFilterMode(const ZoomFilterMode filterMode)
{
  m_filterModes.push_back(static_cast<uint8_t>(filterMode));
}

inline void GoomStateDump::CumulativeState::AddCurrentHypercosOverlay(
    const HypercosOverlay hypercosOverlay)
{
  m_hypercosOverlays.push_back(static_cast<uint8_t>(hypercosOverlay));
}

inline void GoomStateDump::CumulativeState::AddCurrentBlockyWavyEffect(const bool value)
{
  m_blockyWavyEffects.push_back(static_cast<uint8_t>(value));
}

inline void GoomStateDump::CumulativeState::AddCurrentImageVelocityEffect(const bool value)
{
  m_imageVelocityEffects.push_back(static_cast<uint8_t>(value));
}

inline void GoomStateDump::CumulativeState::AddCurrentNoiseEffect(const bool value)
{
  m_noiseEffects.push_back(static_cast<uint8_t>(value));
}

inline void GoomStateDump::CumulativeState::AddCurrentPlaneEffect(const bool value)
{
  m_planeEffects.push_back(static_cast<uint8_t>(value));
}

inline void GoomStateDump::CumulativeState::AddCurrentRotationEffect(const bool value)
{
  m_rotationEffects.push_back(static_cast<uint8_t>(value));
}

inline void GoomStateDump::CumulativeState::AddCurrentTanEffect(const bool value)
{
  m_tanEffects.push_back(static_cast<uint8_t>(value));
}

inline void GoomStateDump::CumulativeState::AddCurrentTimeSinceLastGoom(const uint32_t value)
{
  m_timeSinceLastGoom.push_back(value);
}

inline void GoomStateDump::CumulativeState::AddCurrentTimeSinceLastBigGoom(const uint32_t value)
{
  m_timeSinceLastBigGoom.push_back(value);
}

inline void GoomStateDump::CumulativeState::AddCurrentTotalGoomsInCurrentCycle(const uint32_t value)
{
  m_totalGoomsInCurrentCycle.push_back(value);
}

inline void GoomStateDump::CumulativeState::AddCurrentGoomPower(const float value)
{
  m_goomPower.push_back(value);
}

inline void GoomStateDump::CumulativeState::AddCurrentGoomVolume(const float value)
{
  m_goomVolume.push_back(value);
}

inline auto GoomStateDump::CumulativeState::GetNumUpdates() const -> uint32_t
{
  return m_updateNum;
}

inline auto GoomStateDump::CumulativeState::GetUpdateTimesInMs() const
    -> const std::vector<uint32_t>&
{
  return m_updateTimesInMs;
}

inline auto GoomStateDump::CumulativeState::GetGoomStates() const -> const std::vector<uint8_t>&
{
  return m_goomStates;
}

inline auto GoomStateDump::CumulativeState::GetFilterModes() const -> const std::vector<uint8_t>&
{
  return m_filterModes;
}

inline auto GoomStateDump::CumulativeState::GetHypercosOverlays() const
    -> const std::vector<uint8_t>&
{
  return m_hypercosOverlays;
}

inline auto GoomStateDump::CumulativeState::GetBlockyWavyEffects() const
    -> const std::vector<uint8_t>&
{
  return m_blockyWavyEffects;
}

inline auto GoomStateDump::CumulativeState::GetImageVelocityEffects() const
    -> const std::vector<uint8_t>&
{
  return m_imageVelocityEffects;
}

inline auto GoomStateDump::CumulativeState::GetNoiseEffects() const -> const std::vector<uint8_t>&
{
  return m_noiseEffects;
}

inline auto GoomStateDump::CumulativeState::GetPlaneEffects() const -> const std::vector<uint8_t>&
{
  return m_planeEffects;
}

inline auto GoomStateDump::CumulativeState::GetRotationEffects() const
    -> const std::vector<uint8_t>&
{
  return m_rotationEffects;
}

inline auto GoomStateDump::CumulativeState::GetTanEffects() const -> const std::vector<uint8_t>&
{
  return m_tanEffects;
}

inline auto GoomStateDump::CumulativeState::GetTimeSinceLastGoom() const
    -> const std::vector<uint32_t>&
{
  return m_timeSinceLastGoom;
}

inline auto GoomStateDump::CumulativeState::GetTimeSinceLastBigGoom() const
    -> const std::vector<uint32_t>&
{
  return m_timeSinceLastBigGoom;
}

inline auto GoomStateDump::CumulativeState::GetTotalGoomsInCurrentCycle() const
    -> const std::vector<uint32_t>&
{
  return m_totalGoomsInCurrentCycle;
}

inline auto GoomStateDump::CumulativeState::GetGoomPower() const -> const std::vector<float>&
{
  return m_goomPower;
}

inline auto GoomStateDump::CumulativeState::GetGoomVolume() const -> const std::vector<float>&
{
  return m_goomVolume;
}

} // namespace GOOM::CONTROL
