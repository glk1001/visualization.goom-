#pragma once

#include "../filters/amulet.h"
#include "../filters/crystal_ball.h"
#include "../filters/filter_buffers.h"
#include "../filters/hypercos.h"
#include "../filters/image_speed_coeffs.h"
#include "../filters/planes.h"
#include "../filters/scrunch.h"
#include "../filters/speedway.h"
#include "../filters/wave.h"
#include "../filters/y_only.h"
#include "filters/filter_settings.h"
#include "goom/goom_config.h"
#include "goom/goom_stats.h"
#include "goomutils/enumutils.h"

#include <array>
#include <chrono>
#include <cstdint>
#include <string>

namespace GOOM
{

class FilterStats
{
public:
  FilterStats() = default;

  void Reset();
  void Log(const GoomStats::LogStatsValueFunc& l) const;

  void SetLastZoomFilterSettings(const ZoomFilterData& filterSettings);
  void SetLastJustChangedFilterSettings(bool val);
  void SetLastAmuletParams(const FILTERS::Amulet::Params& params);
  void SetLastCrystalBallParams(const FILTERS::CrystalBall::Params& params);
  void SetLastHypercosParams(const FILTERS::Hypercos::Params& params);
  void SetLastImageDisplacementsFilename(const std::string& imageFilename);
  void SetLastImageDisplacementsParams(const FILTERS::ImageDisplacements::Params& params);
  void SetLastPlanesParams(const FILTERS::Planes::Params& params);
  void SetLastScrunchParams(const FILTERS::Scrunch::Params& params);
  void SetLastSpeedwayParams(const FILTERS::Speedway::Params& params);
  void SetLastWaveParams(const FILTERS::Wave::Params& params);
  void SetLastYOnlyParams(const FILTERS::YOnly::Params& params);
  void SetLastGeneralSpeed(float val);
  void SetLastPrevX(uint32_t val);
  void SetLastPrevY(uint32_t val);
  void SetLastTranBuffYLineStart(uint32_t val);
  void SetLastTranDiffFactor(int val);

  void UpdateStart();
  void UpdateEnd();

  void UpdateTranBuffersStart();
  void UpdateTranBuffersEnd(ZoomFilterMode mode,
                            FILTERS::ZoomFilterBuffers::TranBuffersState bufferState);

  void DoChangeFilterSettings(const ZoomFilterData& filterSettings);
  void DoZoomFilterFastRgb();
  void DoCZoom();
  void DoResetTranBuffers();
  void DoStartFreshTranBuffers();
  void DoSwitchIncrNotZero();
  void DoSwitchMultNotOne();
  void DoTranPointClipped();
  void DoZoomVectorNoisify();
  void DoZoomVectorNoiseFactor();
  void DoZoomVectorHypercosOverlay();
  void DoZoomVectorHPlaneEffect();
  void DoZoomVectorVPlaneEffect();
  void DoZoomVectorTanEffect();
  void DoZoomVectorNegativeRotate();
  void DoZoomVectorPositiveRotate();
  void DoZoomVectorSpeedCoeffBelowMin();
  void DoZoomVectorSpeedCoeffAboveMax();

private:
  using TimePoint =
      std::chrono::time_point<std::chrono::high_resolution_clock, std::chrono::nanoseconds>;

  uint32_t m_numUpdates = 0;
  uint64_t m_totalTimeInUpdatesMs = 0;
  uint32_t m_minTimeInUpdatesMs = std::numeric_limits<uint32_t>::max();
  uint32_t m_maxTimeInUpdatesMs = 0;
  TimePoint m_startUpdateTime{};

  uint32_t m_numTranBuffersUpdates = 0;
  uint64_t m_totalTimeInTranBuffersUpdatesMs = 0;
  uint32_t m_minTimeInTranBuffersUpdatesMs = std::numeric_limits<uint32_t>::max();
  uint32_t m_maxTimeInTranBuffersUpdatesMs = 0;
  TimePoint m_startTranBuffersUpdateTime{};
  ZoomFilterMode m_modeAtMinTimeOfTranBuffersUpdate{ZoomFilterMode::_NULL};
  FILTERS::ZoomFilterBuffers::TranBuffersState m_bufferStateAtMinTimeOfTranBuffersUpdate{
      FILTERS::ZoomFilterBuffers::TranBuffersState::_NULL};
  ZoomFilterMode m_modeAtMaxTimeOfTranBuffersUpdate{ZoomFilterMode::_NULL};
  FILTERS::ZoomFilterBuffers::TranBuffersState m_bufferStateAtMaxTimeOfTranBuffersUpdate{
      FILTERS::ZoomFilterBuffers::TranBuffersState::_NULL};

  std::array<uint32_t, UTILS::NUM<ZoomFilterMode>> m_numUpdatesInMode{0};
  bool m_lastJustChangedFilterSettings = false;
  float m_lastGeneralSpeed = -1000.0;
  uint32_t m_lastPrevX = 0;
  uint32_t m_lastPrevY = 0;
  uint32_t m_lastTranBuffYLineStart = +1000000;
  int32_t m_lastTranDiffFactor = -1000;
  ZoomFilterData m_lastZoomFilterSettings{};
  FILTERS::Amulet::Params m_lastAmuletParams{};
  FILTERS::CrystalBall::Params m_lastCrystalBallParams{};
  FILTERS::Hypercos::Params m_lastHypercosParams{};
  std::string m_lastImageDisplacementFilename{};
  FILTERS::ImageDisplacements::Params m_lastImageDisplacementsParams{};
  FILTERS::Planes::Params m_lastPlanesParams{};
  FILTERS::Scrunch::Params m_lastScrunchParams{};
  FILTERS::Speedway::Params m_lastSpeedwayParams{};
  FILTERS::Wave::Params m_lastWaveParams{};
  FILTERS::YOnly::Params m_lastYOnlyParams{};

  uint32_t m_numChangeFilterSettings = 0;
  uint64_t m_numZoomFilterFastRgb = 0;
  uint64_t m_numStartFreshTranBuffers = 0;
  uint64_t m_numResetTranBuffers = 0;
  uint64_t m_numSwitchIncrNotZero = 0;
  uint64_t m_numSwitchMultNotOne = 0;
  uint64_t m_numZoomVectorTanEffect = 0;
  uint64_t m_numZoomVectorNegativeRotate = 0;
  uint64_t m_numZoomVectorPositiveRotate = 0;
  uint64_t m_numTranPointsClipped = 0;
  uint64_t m_numZoomVectors = 0;
  uint64_t m_numZoomVectorNoisify = 0;
  uint64_t m_numZoomVectorChangeNoiseFactor = 0;
  uint64_t m_numZoomVectorHypercosOverlay = 0;
  uint64_t m_numZoomVectorHPlaneEffect = 0;
  uint64_t m_numZoomVectorVPlaneEffect = 0;
  uint64_t m_numCZoom = 0;
  uint64_t m_numZoomVectorCoeffVitesseBelowMin = 0;
  uint64_t m_numZoomVectorCoeffVitesseAboveMax = 0;
};

} // namespace GOOM

