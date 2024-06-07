// #define DO_GOOM_STATE_DUMP

module;

#undef NO_LOGGING // NOLINT: This maybe be defined on command line.

#include "goom/goom_logger.h"

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <format>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

module Goom.Control.GoomStateDump;

#ifdef DO_GOOM_STATE_DUMP

import Goom.Control.GoomAllVisualFx;
import Goom.Control.GoomMusicSettingsReactor;
import Goom.Control.GoomStates;
import Goom.FilterFx.AfterEffects.AfterEffectsStates;
import Goom.FilterFx.AfterEffects.AfterEffectsTypes;
import Goom.FilterFx.FilterSettingsService;
import Goom.Utils.DateUtils;
import Goom.Lib.GoomControl;

namespace GOOM::CONTROL
{

using FILTER_FX::FilterSettingsService;
using FILTER_FX::ZoomFilterMode;
using FILTER_FX::AFTER_EFFECTS::HypercosOverlayMode;
using UTILS::GetCurrentDateTimeAsString;

class GoomStateDump::CumulativeState
{
public:
  CumulativeState() noexcept = default;

  auto Reset() noexcept -> void;
  auto IncrementUpdateNum() noexcept -> void;

  [[nodiscard]] auto GetNumUpdates() const -> uint32_t;
  [[nodiscard]] auto GetGoomTimes() const -> const std::vector<uint64_t>&;

  auto AddCurrentGoomTime(uint64_t goomTime) noexcept -> void;
  auto AddCurrentUpdateTime(uint32_t timeInMs) noexcept -> void;

  auto AddCurrentGoomState(GoomStates goomState) noexcept -> void;
  auto AddCurrentFilterMode(ZoomFilterMode filterMode) noexcept -> void;
  auto AddCurrentHypercosOverlay(HypercosOverlayMode hypercosOverlay) noexcept -> void;
  auto AddCurrentImageVelocityEffect(bool value) noexcept -> void;
  auto AddCurrentNoiseEffect(bool value) noexcept -> void;
  auto AddCurrentPlaneEffect(bool value) noexcept -> void;
  auto AddCurrentRotationEffect(bool value) noexcept -> void;
  auto AddCurrentTanEffect(bool value) noexcept -> void;
  auto AddCurrentXYLerpEffect(bool value) noexcept -> void;

  auto AddTransformBufferLerpFactor(float value) noexcept -> void;

  auto AddCurrentTimeSinceLastGoom(uint32_t value) noexcept -> void;
  auto AddCurrentTimeSinceLastBigGoom(uint32_t value) noexcept -> void;
  auto AddCurrentTotalGoomsInCurrentCycle(uint32_t value) noexcept -> void;
  auto AddCurrentGoomPower(float value) noexcept -> void;
  auto AddCurrentGoomVolume(float value) noexcept -> void;

  [[nodiscard]] auto GetUpdateTimesInMs() const -> const std::vector<uint32_t>&;

  [[nodiscard]] auto GetGoomStates() const -> const std::vector<uint8_t>&;
  [[nodiscard]] auto GetFilterModes() const -> const std::vector<uint8_t>&;
  [[nodiscard]] auto GetHypercosOverlays() const -> const std::vector<uint8_t>&;
  [[nodiscard]] auto GetImageVelocityEffects() const -> const std::vector<uint8_t>&;
  [[nodiscard]] auto GetNoiseEffects() const -> const std::vector<uint8_t>&;
  [[nodiscard]] auto GetPlaneEffects() const -> const std::vector<uint8_t>&;
  [[nodiscard]] auto GetRotationEffects() const -> const std::vector<uint8_t>&;
  [[nodiscard]] auto GetTanEffects() const -> const std::vector<uint8_t>&;
  [[nodiscard]] auto GetXYLerpEffects() const -> const std::vector<uint8_t>&;

  [[nodiscard]] auto GetTransformBufferLerpFactors() const -> const std::vector<float>&;

  [[nodiscard]] auto GetTimesSinceLastGoom() const -> const std::vector<uint32_t>&;
  [[nodiscard]] auto GetTimesSinceLastBigGoom() const -> const std::vector<uint32_t>&;
  [[nodiscard]] auto GetTotalGoomsInCurrentCycle() const -> const std::vector<uint32_t>&;
  [[nodiscard]] auto GetGoomPowers() const -> const std::vector<float>&;
  [[nodiscard]] auto GetGoomVolumes() const -> const std::vector<float>&;

private:
  static constexpr uint32_t INITIAL_NUM_UPDATES_ESTIMATE = 5 * 60 * 60 * 25;
  static constexpr uint32_t EXTRA_NUM_UPDATES_ESTIMATE   = 1 * 60 * 60 * 25;
  uint32_t m_numUpdatesEstimate                          = INITIAL_NUM_UPDATES_ESTIMATE;
  uint32_t m_updateNum                                   = 0;

  std::vector<uint64_t> m_goomTimes{};
  std::vector<uint32_t> m_updateTimesInMs{};
  std::vector<uint8_t> m_goomStates{};
  std::vector<uint8_t> m_filterModes{};
  std::vector<uint8_t> m_hypercosOverlays{};
  std::vector<uint8_t> m_imageVelocityEffects{};
  std::vector<uint8_t> m_noiseEffects{};
  std::vector<uint8_t> m_planeEffects{};
  std::vector<uint8_t> m_rotationEffects{};
  std::vector<uint8_t> m_tanEffects{};
  std::vector<uint8_t> m_xyLerpEffects{};

  std::vector<float> m_bufferLerps{};

  std::vector<uint32_t> m_timesSinceLastGoom{};
  std::vector<uint32_t> m_timesSinceLastBigGoom{};
  std::vector<uint32_t> m_totalGoomsInCurrentCycle{};
  std::vector<float> m_goomPowers{};
  std::vector<float> m_goomVolumes{};

  auto Reserve() noexcept -> void;
};

GoomStateDump::GoomStateDump(const PluginInfo& goomInfo,
                             GoomLogger& goomLogger,
                             const GoomControl& goomControl,
                             const GoomAllVisualFx& visualFx,
                             [[maybe_unused]] const GoomMusicSettingsReactor& musicSettingsReactor,
                             const FilterSettingsService& filterSettingsService) noexcept
  : m_goomInfo{&goomInfo},
    m_goomLogger{&goomLogger},
    m_goomControl{&goomControl},
    m_visualFx{&visualFx},
    //    m_musicSettingsReactor{musicSettingsReactor},
    m_filterSettingsService{&filterSettingsService},
    m_cumulativeState{std::make_unique<CumulativeState>()}
{
}

auto GoomStateDump::Start() noexcept -> void
{
  m_cumulativeState->Reset();

  m_prevTimeHiRes = std::chrono::high_resolution_clock::now();
}

auto GoomStateDump::AddCurrentState() noexcept -> void
{
  m_cumulativeState->AddCurrentGoomTime(m_goomInfo->GetTime().GetCurrentTime());

  const auto timeNow          = std::chrono::high_resolution_clock::now();
  const auto diff             = std::chrono::duration_cast<Ms>(timeNow - m_prevTimeHiRes);
  const auto timeOfUpdateInMs = static_cast<uint32_t>(diff.count());
  m_cumulativeState->AddCurrentUpdateTime(timeOfUpdateInMs);
  m_prevTimeHiRes = timeNow;

  m_cumulativeState->AddCurrentGoomState(m_visualFx->GetCurrentState());
  m_cumulativeState->AddCurrentFilterMode(m_filterSettingsService->GetCurrentFilterMode());

  m_cumulativeState->AddTransformBufferLerpFactor(
      m_goomControl->GetFrameData().miscData.lerpFactor);

  const auto filterSettings        = m_filterSettingsService->GetFilterSettings();
  using AfterEffects               = FILTER_FX::AFTER_EFFECTS::AfterEffectsTypes;
  const auto& afterEffectsSettings = filterSettings.filterEffectsSettings.afterEffectsSettings;
  m_cumulativeState->AddCurrentHypercosOverlay(afterEffectsSettings.hypercosOverlayMode);
  m_cumulativeState->AddCurrentImageVelocityEffect(
      afterEffectsSettings.isActive[AfterEffects::IMAGE_VELOCITY]);
  m_cumulativeState->AddCurrentNoiseEffect(afterEffectsSettings.isActive[AfterEffects::NOISE]);
  m_cumulativeState->AddCurrentPlaneEffect(afterEffectsSettings.isActive[AfterEffects::PLANES]);
  m_cumulativeState->AddCurrentRotationEffect(
      afterEffectsSettings.isActive[AfterEffects::ROTATION]);
  m_cumulativeState->AddCurrentTanEffect(afterEffectsSettings.isActive[AfterEffects::TAN_EFFECT]);
  m_cumulativeState->AddCurrentXYLerpEffect(
      afterEffectsSettings.isActive[AfterEffects::XY_LERP_EFFECT]);

  const auto& goomSoundEvents = m_goomInfo->GetSoundEvents();
  m_cumulativeState->AddCurrentTimeSinceLastGoom(goomSoundEvents.GetTimeSinceLastGoom());
  m_cumulativeState->AddCurrentTimeSinceLastBigGoom(goomSoundEvents.GetTimeSinceLastBigGoom());
  m_cumulativeState->AddCurrentTotalGoomsInCurrentCycle(
      goomSoundEvents.GetTotalGoomsInCurrentCycle());
  m_cumulativeState->AddCurrentGoomPower(goomSoundEvents.GetGoomPower());
  m_cumulativeState->AddCurrentGoomVolume(goomSoundEvents.GetSoundInfo().GetVolume());

  m_cumulativeState->IncrementUpdateNum();
}

auto GoomStateDump::DumpData(const std::string& directory) -> void
{
  if (m_cumulativeState->GetNumUpdates() < MIN_TIMELINE_ELEMENTS_TO_DUMP)
  {
    LogWarn(*m_goomLogger,
            "Not dumping. Too few goom updates: {} < {}.",
            m_cumulativeState->GetNumUpdates(),
            MIN_TIMELINE_ELEMENTS_TO_DUMP);
    return;
  }

  m_dateTime = GetCurrentDateTimeAsString();

  SetCurrentDatedDirectory(directory);

  DumpSummary();

  DumpDataArray("update_times", m_cumulativeState->GetUpdateTimesInMs());

  DumpDataArray("goom_states", m_cumulativeState->GetGoomStates());
  DumpDataArray("filter_modes", m_cumulativeState->GetFilterModes());
  DumpDataArray("hypercos_overlays", m_cumulativeState->GetHypercosOverlays());
  DumpDataArray("image_velocity_effects", m_cumulativeState->GetImageVelocityEffects());
  DumpDataArray("noise_effects", m_cumulativeState->GetNoiseEffects());
  DumpDataArray("plane_effects", m_cumulativeState->GetPlaneEffects());
  DumpDataArray("rotation_effects", m_cumulativeState->GetRotationEffects());
  DumpDataArray("tan_effects", m_cumulativeState->GetTanEffects());
  DumpDataArray("xyLerp_effects", m_cumulativeState->GetXYLerpEffects());

  DumpDataArray("buffer_lerps", m_cumulativeState->GetTransformBufferLerpFactors());

  DumpDataArray("times_since_last_goom", m_cumulativeState->GetTimesSinceLastGoom());
  DumpDataArray("times_since_last_big_goom", m_cumulativeState->GetTimesSinceLastBigGoom());
  DumpDataArray("total_gooms_in_current_cycle", m_cumulativeState->GetTotalGoomsInCurrentCycle());
  DumpDataArray("goom_powers", m_cumulativeState->GetGoomPowers());
  DumpDataArray("goom_volumes", m_cumulativeState->GetGoomVolumes());
}

auto GoomStateDump::SetCurrentDatedDirectory(const std::string& parentDirectory) -> void
{
  m_datedDirectory = parentDirectory + "/" + m_dateTime;
  LogInfo(*m_goomLogger, "Dumping state data to \"{}\"...", m_datedDirectory);
  std::filesystem::create_directories(m_datedDirectory);
}

auto GoomStateDump::DumpSummary() const -> void
{
  static constexpr auto* SUMMARY_FILENAME = "summary.dat";
  auto out = std::ofstream{m_datedDirectory + "/" + SUMMARY_FILENAME, std::ofstream::out};

  out << std::format("Song:       {}\n", m_songTitle);
  out << std::format("Date:       {}\n", m_dateTime);
  out << std::format("Seed:       {}\n", m_goomSeed);
  out << std::format("Width:      {}\n", m_goomInfo->GetDimensions().GetWidth());
  out << std::format("Height:     {}\n", m_goomInfo->GetDimensions().GetHeight());
  out << std::format("Start Time: {}\n", m_stopwatch->GetStartTimeAsStr());
  out << std::format("Stop Time:  {}\n", m_stopwatch->GetLastMarkedTimeAsStr());
  out << std::format("Act Dur:    {}\n", m_stopwatch->GetActualDurationInMs());
  out << std::format("Given Dur:  {}\n", m_stopwatch->GetDurationInMs());
  out << std::format("Time Left:  {}\n", m_stopwatch->GetTimeValues().timeRemainingInMs);
}

template<typename T>
auto GoomStateDump::DumpDataArray(const std::string& filename,
                                  const std::vector<T>& dataArray) const -> void
{
  const auto dataLen = m_cumulativeState->GetNumUpdates();
  LogInfo(*m_goomLogger, "Dumping Goom state data ({} values) to \"{}\".", dataLen, filename);

  static constexpr auto* EXT = ".dat";
  auto out = std::ofstream{m_datedDirectory + "/" + filename + EXT, std::ofstream::out};

  for (auto i = 0U; i < dataLen; ++i)
  {
    out << GetFormattedRowStr(m_cumulativeState->GetGoomTimes().at(i), dataArray.at(i));
  }
}

template<typename T>
auto GoomStateDump::GetFormattedRowStr(const uint64_t value1, const T value2) -> std::string
{
  return std::format("{:6} {:7}\n", value1, value2);
}

template<>
auto GoomStateDump::GetFormattedRowStr(const uint64_t value1, const float value2) -> std::string
{
  return std::format("{:6} {:7.3f}\n", value1, value2);
}

inline auto GoomStateDump::CumulativeState::Reset() noexcept -> void
{
  m_updateNum = 0;

  Reserve();
}

auto GoomStateDump::CumulativeState::Reserve() noexcept -> void
{
  m_updateTimesInMs.reserve(m_numUpdatesEstimate);
  m_goomStates.reserve(m_numUpdatesEstimate);
  m_filterModes.reserve(m_numUpdatesEstimate);
  m_bufferLerps.reserve(m_numUpdatesEstimate);
  m_hypercosOverlays.reserve(m_numUpdatesEstimate);
  m_imageVelocityEffects.reserve(m_numUpdatesEstimate);
  m_noiseEffects.reserve(m_numUpdatesEstimate);
  m_planeEffects.reserve(m_numUpdatesEstimate);
  m_rotationEffects.reserve(m_numUpdatesEstimate);
  m_tanEffects.reserve(m_numUpdatesEstimate);
  m_xyLerpEffects.reserve(m_numUpdatesEstimate);
}

inline auto GoomStateDump::CumulativeState::IncrementUpdateNum() noexcept -> void
{
  ++m_updateNum;

  if (m_updateNum > m_numUpdatesEstimate)
  {
    m_numUpdatesEstimate += EXTRA_NUM_UPDATES_ESTIMATE;
    Reserve();
  }
}

inline auto GoomStateDump::CumulativeState::AddCurrentGoomTime(const uint64_t goomTime) noexcept
    -> void
{
  m_goomTimes.push_back(goomTime);
}

inline auto GoomStateDump::CumulativeState::AddCurrentUpdateTime(const uint32_t timeInMs) noexcept
    -> void
{
  m_updateTimesInMs.push_back(timeInMs);
}

inline auto GoomStateDump::CumulativeState::AddCurrentGoomState(const GoomStates goomState) noexcept
    -> void
{
  m_goomStates.push_back(static_cast<uint8_t>(goomState));
}

inline auto GoomStateDump::CumulativeState::AddCurrentFilterMode(
    const ZoomFilterMode filterMode) noexcept -> void
{
  m_filterModes.push_back(static_cast<uint8_t>(filterMode));
}

inline auto GoomStateDump::CumulativeState::AddCurrentHypercosOverlay(
    const HypercosOverlayMode hypercosOverlay) noexcept -> void
{
  m_hypercosOverlays.push_back(static_cast<uint8_t>(hypercosOverlay));
}

inline auto GoomStateDump::CumulativeState::AddCurrentImageVelocityEffect(const bool value) noexcept
    -> void
{
  m_imageVelocityEffects.push_back(static_cast<uint8_t>(value));
}

inline auto GoomStateDump::CumulativeState::AddCurrentNoiseEffect(const bool value) noexcept -> void
{
  m_noiseEffects.push_back(static_cast<uint8_t>(value));
}

inline auto GoomStateDump::CumulativeState::AddCurrentPlaneEffect(const bool value) noexcept -> void
{
  m_planeEffects.push_back(static_cast<uint8_t>(value));
}

inline auto GoomStateDump::CumulativeState::AddCurrentRotationEffect(const bool value) noexcept
    -> void
{
  m_rotationEffects.push_back(static_cast<uint8_t>(value));
}

inline auto GoomStateDump::CumulativeState::AddTransformBufferLerpFactor(const float value) noexcept
    -> void
{
  m_bufferLerps.push_back(value);
}

inline auto GoomStateDump::CumulativeState::AddCurrentTanEffect(const bool value) noexcept -> void
{
  m_tanEffects.push_back(static_cast<uint8_t>(value));
}

inline auto GoomStateDump::CumulativeState::AddCurrentXYLerpEffect(const bool value) noexcept
    -> void
{
  m_xyLerpEffects.push_back(static_cast<uint8_t>(value));
}

inline auto GoomStateDump::CumulativeState::AddCurrentTimeSinceLastGoom(
    const uint32_t value) noexcept -> void
{
  m_timesSinceLastGoom.push_back(value);
}

inline auto GoomStateDump::CumulativeState::AddCurrentTimeSinceLastBigGoom(
    const uint32_t value) noexcept -> void
{
  m_timesSinceLastBigGoom.push_back(value);
}

inline auto GoomStateDump::CumulativeState::AddCurrentTotalGoomsInCurrentCycle(
    const uint32_t value) noexcept -> void
{
  m_totalGoomsInCurrentCycle.push_back(value);
}

inline auto GoomStateDump::CumulativeState::AddCurrentGoomPower(const float value) noexcept -> void
{
  m_goomPowers.push_back(value);
}

inline auto GoomStateDump::CumulativeState::AddCurrentGoomVolume(const float value) noexcept -> void
{
  m_goomVolumes.push_back(value);
}

inline auto GoomStateDump::CumulativeState::GetNumUpdates() const -> uint32_t
{
  return m_updateNum;
}

inline auto GoomStateDump::CumulativeState::GetGoomTimes() const -> const std::vector<uint64_t>&
{
  return m_goomTimes;
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

inline auto GoomStateDump::CumulativeState::GetTransformBufferLerpFactors() const
    -> const std::vector<float>&
{
  return m_bufferLerps;
}

inline auto GoomStateDump::CumulativeState::GetTanEffects() const -> const std::vector<uint8_t>&
{
  return m_tanEffects;
}

inline auto GoomStateDump::CumulativeState::GetXYLerpEffects() const -> const std::vector<uint8_t>&
{
  return m_xyLerpEffects;
}

inline auto GoomStateDump::CumulativeState::GetTimesSinceLastGoom() const
    -> const std::vector<uint32_t>&
{
  return m_timesSinceLastGoom;
}

inline auto GoomStateDump::CumulativeState::GetTimesSinceLastBigGoom() const
    -> const std::vector<uint32_t>&
{
  return m_timesSinceLastBigGoom;
}

inline auto GoomStateDump::CumulativeState::GetTotalGoomsInCurrentCycle() const
    -> const std::vector<uint32_t>&
{
  return m_totalGoomsInCurrentCycle;
}

inline auto GoomStateDump::CumulativeState::GetGoomPowers() const -> const std::vector<float>&
{
  return m_goomPowers;
}

inline auto GoomStateDump::CumulativeState::GetGoomVolumes() const -> const std::vector<float>&
{
  return m_goomVolumes;
}

} // namespace GOOM::CONTROL

#endif // DO_GOOM_STATE_DUMP
