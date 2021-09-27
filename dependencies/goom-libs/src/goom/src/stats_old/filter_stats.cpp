#include "filter_stats.h"

#include "../filters/image_displacement.h"
#include "filters/filter_settings.h"
#include "goom/goom_stats.h"
#include "goomutils/enumutils.h"
#include "goomutils/graphics/image_bitmaps.h"

#include <chrono>
#include <cmath>
#include <cstdint>


namespace GOOM
{

using FILTERS::ZoomFilterBuffers;
using UTILS::EnumToString;

void FilterStats::Reset()
{
  m_numUpdates = 0;
  m_totalTimeInUpdatesMs = 0;
  m_minTimeInUpdatesMs = std::numeric_limits<uint32_t>::max();
  m_maxTimeInUpdatesMs = 0;
  m_startUpdateTime = std::chrono::high_resolution_clock::now();

  m_numTranBuffersUpdates = 0;
  m_totalTimeInTranBuffersUpdatesMs = 0;
  m_minTimeInTranBuffersUpdatesMs = std::numeric_limits<uint32_t>::max();
  m_maxTimeInTranBuffersUpdatesMs = 0;
  m_startTranBuffersUpdateTime = std::chrono::high_resolution_clock::now();
  m_modeAtMinTimeOfTranBuffersUpdate = ZoomFilterMode::_NULL;
  m_bufferStateAtMinTimeOfTranBuffersUpdate = ZoomFilterBuffers::TranBuffersState::_NULL;
  m_modeAtMaxTimeOfTranBuffersUpdate = ZoomFilterMode::_NULL;
  m_bufferStateAtMaxTimeOfTranBuffersUpdate = ZoomFilterBuffers::TranBuffersState::_NULL;

  std::fill(m_numUpdatesInMode.begin(), m_numUpdatesInMode.end(), 0);

  m_numChangeFilterSettings = 0;
  m_numZoomVectors = 0;
  m_numZoomVectorNoisify = 0;
  m_numZoomVectorChangeNoiseFactor = 0;
  m_numZoomVectorHypercosOverlay = 0;
  m_numZoomVectorHPlaneEffect = 0;
  m_numZoomVectorVPlaneEffect = 0;
  m_numCZoom = 0;
  m_numZoomFilterFastRgb = 0;
  m_numStartFreshTranBuffers = 0;
  m_numResetTranBuffers = 0;
  m_numSwitchIncrNotZero = 0;
  m_numSwitchMultNotOne = 0;
  m_numZoomVectorTanEffect = 0;
  m_numZoomVectorNegativeRotate = 0;
  m_numZoomVectorPositiveRotate = 0;
  m_numTranPointsClipped = 0;
  m_numZoomVectorCoeffVitesseBelowMin = 0;
  m_numZoomVectorCoeffVitesseAboveMax = 0;
}

void FilterStats::Log(const GoomStats::LogStatsValueFunc& logVal) const
{
  const constexpr char* MODULE = "Filter";

  const auto avTimeInUpdateMs = static_cast<int32_t>(std::lround(
      m_numUpdates == 0
          ? -1.0
          : static_cast<float>(m_totalTimeInUpdatesMs) / static_cast<float>(m_numUpdates)));
  logVal(MODULE, "avTimeInUpdateMs", avTimeInUpdateMs);
  logVal(MODULE, "minTimeInUpdatesMs", m_minTimeInUpdatesMs);
  logVal(MODULE, "maxTimeInUpdatesMs", m_maxTimeInUpdatesMs);

  const auto avTimeInTranBuffersUpdateMs = static_cast<int32_t>(std::lround(
      m_numTranBuffersUpdates == 0 ? -1.0
                                   : static_cast<float>(m_totalTimeInTranBuffersUpdatesMs) /
                                         static_cast<float>(m_numTranBuffersUpdates)));
  logVal(MODULE, "avTimeInTranBuffersUpdateMs", avTimeInTranBuffersUpdateMs);
  logVal(MODULE, "minTimeInTranBuffersUpdatesMs", m_minTimeInTranBuffersUpdatesMs);
  logVal(MODULE, "maxTimeInTranBuffersUpdatesMs", m_maxTimeInTranBuffersUpdatesMs);
  logVal(MODULE, "m_modeAtMinTimeOfTranBuffersUpdate",
         EnumToString(m_modeAtMinTimeOfTranBuffersUpdate));
  logVal(MODULE, "m_modeAtMaxTimeOfTranBuffersUpdate",
         EnumToString(m_modeAtMaxTimeOfTranBuffersUpdate));
  logVal(MODULE, "m_bufferStateAtMinTimeOfTranBuffersUpdate",
         EnumToString(m_bufferStateAtMinTimeOfTranBuffersUpdate));
  logVal(MODULE, "m_bufferStateAtMaxTimeOfTranBuffersUpdate",
         EnumToString(m_bufferStateAtMaxTimeOfTranBuffersUpdate));

  logVal(MODULE, "lastZoomFilterData.mode", EnumToString(m_lastZoomFilterSettings.mode));
  logVal(MODULE, "lastJustChangedFilterSettings",
         static_cast<uint32_t>(m_lastJustChangedFilterSettings));
  logVal(MODULE, "lastGeneralSpeed", m_lastGeneralSpeed);
  logVal(MODULE, "lastPrevX", m_lastPrevX);
  logVal(MODULE, "lastPrevY", m_lastPrevY);
  logVal(MODULE, "lastTranBuffYLineStart", m_lastTranBuffYLineStart);
  logVal(MODULE, "lastTranDiffFactor", m_lastTranDiffFactor);

  logVal(MODULE, "lastZoomFilterData->vitesse", m_lastZoomFilterSettings.vitesse.GetVitesse());
  logVal(MODULE, "lastZoomFilterData->reverseSpeed",
         static_cast<uint32_t>(m_lastZoomFilterSettings.vitesse.GetReverseVitesse()));
  logVal(MODULE, "lastZoomFilterData->relativeSpeed",
         m_lastZoomFilterSettings.vitesse.GetRelativeSpeed());
  logVal(MODULE, "lastZoomFilterData->zoomMidPoint.x", m_lastZoomFilterSettings.zoomMidPoint.x);
  logVal(MODULE, "lastZoomFilterData->zoomMidPoint.y", m_lastZoomFilterSettings.zoomMidPoint.y);

  logVal(MODULE, "lastXAmuletAmplitude", m_lastAmuletParams.xAmplitude);
  logVal(MODULE, "lastYAmuletAmplitude", m_lastAmuletParams.yAmplitude);
  logVal(MODULE, "lastXCrystalBallAmplitude", m_lastCrystalBallParams.xAmplitude);
  logVal(MODULE, "lastYCrystalBallAmplitude", m_lastCrystalBallParams.yAmplitude);
  logVal(MODULE, "lastXCrystalBallSqDistMult", m_lastCrystalBallParams.xSqDistMult);
  logVal(MODULE, "lastYCrystalBallSqDistMult", m_lastCrystalBallParams.ySqDistMult);
  logVal(MODULE, "lastXCrystalBallSqDistOffset", m_lastCrystalBallParams.xSqDistOffset);
  logVal(MODULE, "lastYCrystalBallSqDistOffset", m_lastCrystalBallParams.ySqDistOffset);
  logVal(MODULE, "lastHorizontalPlaneEffect", m_lastPlanesParams.horizontalEffect);
  logVal(MODULE, "lastVerticalPlaneEffect", m_lastPlanesParams.verticalEffect);
  logVal(MODULE, "lastHorizontalPlaneEffectAmplitude",
         m_lastPlanesParams.horizontalEffectAmplitude);
  logVal(MODULE, "lastVerticalPlaneEffectAmplitude", m_lastPlanesParams.verticalEffectAmplitude);
  logVal(MODULE, "lastHypercosEffect", EnumToString(m_lastHypercosParams.effect));
  logVal(MODULE, "lastHypercosReverse", static_cast<uint32_t>(m_lastHypercosParams.reverse));
  logVal(MODULE, "lastHypercosXFreq", m_lastHypercosParams.xFreq);
  logVal(MODULE, "lastHypercosYFreq", m_lastHypercosParams.yFreq);
  logVal(MODULE, "lastHypercosXAmplitude", m_lastHypercosParams.xAmplitude);
  logVal(MODULE, "lastHypercosYAmplitude", m_lastHypercosParams.yAmplitude);
  logVal(MODULE, "lastImageDisplacementFilename", m_lastImageDisplacementFilename);
  logVal(MODULE, "lastImageDisplacementAmplitude", m_lastImageDisplacementsParams.amplitude);
  logVal(MODULE, "lastImageDisplacementXColorCutoff", m_lastImageDisplacementsParams.xColorCutoff);
  logVal(MODULE, "lastImageDisplacementYColorCutoff", m_lastImageDisplacementsParams.yColorCutoff);
  logVal(MODULE, "lastImageDisplacementZoomFactor", m_lastImageDisplacementsParams.zoomFactor);
  logVal(MODULE, "lastXScrunchAmplitude", m_lastScrunchParams.xAmplitude);
  logVal(MODULE, "lastYScrunchAmplitude", m_lastScrunchParams.yAmplitude);
  logVal(MODULE, "lastXSpeedwayAmplitude", m_lastSpeedwayParams.xAmplitude);
  logVal(MODULE, "lastYSpeedwayAmplitude", m_lastSpeedwayParams.yAmplitude);
  logVal(MODULE, "lastXWaveEffect", EnumToString(m_lastWaveParams.xWaveEffect));
  logVal(MODULE, "lastYWaveEffect", EnumToString(m_lastWaveParams.yWaveEffect));
  logVal(MODULE, "lastWaveFreqFactor", m_lastWaveParams.freqFactor);
  logVal(MODULE, "lastWaveAmplitude", m_lastWaveParams.amplitude);
  logVal(MODULE, "lastYOnlyXEffect", EnumToString(m_lastYOnlyParams.xEffect));
  logVal(MODULE, "lastYOnlyYEffect", EnumToString(m_lastYOnlyParams.yEffect));
  logVal(MODULE, "lastYOnlyXFreqFactor", m_lastYOnlyParams.xFreqFactor);
  logVal(MODULE, "lastYOnlyYFreqFactor", m_lastYOnlyParams.yFreqFactor);
  logVal(MODULE, "lastYOnlyXAmplitude", m_lastYOnlyParams.xAmplitude);
  logVal(MODULE, "lastYOnlyYAmplitude", m_lastYOnlyParams.yAmplitude);

  logVal(MODULE, "lastZoomFilterData->noisify",
         static_cast<uint32_t>(m_lastZoomFilterSettings.noisify));
  logVal(MODULE, "lastZoomFilterData->noiseFactor",
         static_cast<float>(m_lastZoomFilterSettings.noiseFactor));
  logVal(MODULE, "lastZoomFilterData->blockyWavy",
         static_cast<uint32_t>(m_lastZoomFilterSettings.blockyWavy));
  logVal(MODULE, "lastZoomFilterData->rotateSpeed", m_lastZoomFilterSettings.rotateSpeed);
  logVal(MODULE, "lastZoomFilterData->tanEffect",
         static_cast<uint32_t>(m_lastZoomFilterSettings.tanEffect));

  for (size_t i = 0; i < m_numUpdatesInMode.size(); ++i)
  {
    const std::string modeStr = EnumToString(static_cast<ZoomFilterMode>(i));
    logVal(MODULE, std20::format("num {} updates", modeStr), m_numUpdatesInMode[i]);
  }

  logVal(MODULE, "numResetTranBuffers", m_numResetTranBuffers);
  logVal(MODULE, "numStartFreshTranBuffers", m_numStartFreshTranBuffers);
  logVal(MODULE, "numSwitchIncrNotZero", m_numSwitchIncrNotZero);
  logVal(MODULE, "numSwitchMultNotOne", m_numSwitchMultNotOne);
  logVal(MODULE, "numTranPointsClipped", m_numTranPointsClipped);

  logVal(MODULE, "numChangeFilterSettings", m_numChangeFilterSettings);
  logVal(MODULE, "numCZoom", m_numCZoom);
  logVal(MODULE, "numZoomVectors", m_numZoomVectors);
  logVal(MODULE, "numZoomVectorNoisify", m_numZoomVectorNoisify);
  logVal(MODULE, "numZoomVectorChangeNoiseFactor", m_numZoomVectorChangeNoiseFactor);
  logVal(MODULE, "numZoomVectorHypercosOverlay", m_numZoomVectorHypercosOverlay);
  logVal(MODULE, "numZoomVectorHPlaneEffect", m_numZoomVectorHPlaneEffect);
  logVal(MODULE, "numZoomVectorVPlaneEffect", m_numZoomVectorVPlaneEffect);
  logVal(MODULE, "numZoomVectorNegativeRotate", m_numZoomVectorNegativeRotate);
  logVal(MODULE, "numZoomVectorPositiveRotate", m_numZoomVectorPositiveRotate);
  logVal(MODULE, "numZoomVectorTanEffect", m_numZoomVectorTanEffect);
  logVal(MODULE, "numZoomVectorCoeffVitesseBelowMin", m_numZoomVectorCoeffVitesseBelowMin);
  logVal(MODULE, "numZoomVectorCoeffVitesseAboveMax", m_numZoomVectorCoeffVitesseAboveMax);
}

void FilterStats::UpdateStart()
{
  m_startUpdateTime = std::chrono::high_resolution_clock::now();
  m_numUpdates++;
}

void FilterStats::UpdateEnd()
{
  const auto timeNow = std::chrono::high_resolution_clock::now();

  using Ms = std::chrono::milliseconds;
  const Ms diff = std::chrono::duration_cast<Ms>(timeNow - m_startUpdateTime);
  const auto timeInUpdateMs = static_cast<uint32_t>(diff.count());
  if (timeInUpdateMs < m_minTimeInUpdatesMs)
  {
    m_minTimeInUpdatesMs = timeInUpdateMs;
  }
  else if (timeInUpdateMs > m_maxTimeInUpdatesMs)
  {
    m_maxTimeInUpdatesMs = timeInUpdateMs;
  }
  m_totalTimeInUpdatesMs += timeInUpdateMs;
}

void FilterStats::UpdateTranBuffersStart()
{
  m_startTranBuffersUpdateTime = std::chrono::high_resolution_clock::now();
  m_numTranBuffersUpdates++;
}

void FilterStats::UpdateTranBuffersEnd(ZoomFilterMode mode,
                                       FILTERS::ZoomFilterBuffers::TranBuffersState bufferState)
{
  const auto timeNow = std::chrono::high_resolution_clock::now();

  using Ms = std::chrono::milliseconds;
  const Ms diff = std::chrono::duration_cast<Ms>(timeNow - m_startTranBuffersUpdateTime);
  const auto timeInUpdateMs = static_cast<uint32_t>(diff.count());
  if (timeInUpdateMs < m_minTimeInTranBuffersUpdatesMs)
  {
    m_minTimeInTranBuffersUpdatesMs = timeInUpdateMs;
    m_modeAtMinTimeOfTranBuffersUpdate = mode;
    m_bufferStateAtMinTimeOfTranBuffersUpdate = bufferState;
  }
  else if (timeInUpdateMs > m_maxTimeInTranBuffersUpdatesMs)
  {
    m_maxTimeInTranBuffersUpdatesMs = timeInUpdateMs;
    m_modeAtMaxTimeOfTranBuffersUpdate = mode;
    m_bufferStateAtMaxTimeOfTranBuffersUpdate = bufferState;
  }
  m_totalTimeInTranBuffersUpdatesMs += timeInUpdateMs;
}

void FilterStats::DoChangeFilterSettings(const ZoomFilterData& filterSettings)
{
  ++m_numChangeFilterSettings;
  ++m_numUpdatesInMode.at(static_cast<size_t>(filterSettings.mode));
}

void FilterStats::DoZoomVectorNoisify()
{
  m_numZoomVectorNoisify++;
}

void FilterStats::DoZoomVectorNoiseFactor()
{
  m_numZoomVectorChangeNoiseFactor++;
}

void FilterStats::DoZoomVectorHypercosOverlay()
{
  m_numZoomVectorHypercosOverlay++;
}

void FilterStats::DoZoomVectorHPlaneEffect()
{
  m_numZoomVectorHPlaneEffect++;
}

void FilterStats::DoZoomVectorVPlaneEffect()
{
  m_numZoomVectorVPlaneEffect++;
}

void FilterStats::DoCZoom()
{
  m_numCZoom++;
}

void FilterStats::DoZoomFilterFastRgb()
{
  m_numZoomFilterFastRgb++;
}

void FilterStats::DoResetTranBuffers()
{
  m_numResetTranBuffers++;
}

void FilterStats::DoStartFreshTranBuffers()
{
  m_numStartFreshTranBuffers++;
}

void FilterStats::DoSwitchMultNotOne()
{
  m_numSwitchMultNotOne++;
}

void FilterStats::DoSwitchIncrNotZero()
{
  m_numSwitchIncrNotZero++;
}

void FilterStats::DoZoomVectorTanEffect()
{
  m_numZoomVectorTanEffect++;
}

void FilterStats::DoZoomVectorNegativeRotate()
{
  m_numZoomVectorNegativeRotate++;
}

void FilterStats::DoZoomVectorPositiveRotate()
{
  m_numZoomVectorPositiveRotate++;
}

void FilterStats::DoTranPointClipped()
{
  m_numTranPointsClipped++;
}

void FilterStats::DoZoomVectorSpeedCoeffBelowMin()
{
  m_numZoomVectorCoeffVitesseBelowMin++;
}

void FilterStats::DoZoomVectorSpeedCoeffAboveMax()
{
  m_numZoomVectorCoeffVitesseAboveMax++;
}

void FilterStats::SetLastZoomFilterSettings(const ZoomFilterData& filterSettings)
{
  m_lastZoomFilterSettings = filterSettings;
}

void FilterStats::SetLastJustChangedFilterSettings(const bool val)
{
  m_lastJustChangedFilterSettings = val;
}

void FilterStats::SetLastAmuletParams(const FILTERS::Amulet::Params& params)
{
  m_lastAmuletParams = params;
}

void FilterStats::SetLastCrystalBallParams(const FILTERS::CrystalBall::Params& params)
{
  m_lastCrystalBallParams = params;
}

void FilterStats::SetLastHypercosParams(const FILTERS::Hypercos::Params& params)
{
  m_lastHypercosParams = params;
}

void FilterStats::SetLastImageDisplacementsFilename(const std::string& imageFilename)
{
  m_lastImageDisplacementFilename = imageFilename;
}

void FilterStats::SetLastImageDisplacementsParams(const FILTERS::ImageDisplacements::Params& params)
{
  m_lastImageDisplacementsParams = params;
}

void FilterStats::SetLastPlanesParams(const FILTERS::Planes::Params& params)
{
  m_lastPlanesParams = params;
}

void FilterStats::SetLastScrunchParams(const FILTERS::Scrunch::Params& params)
{
  m_lastScrunchParams = params;
}

void FilterStats::SetLastSpeedwayParams(const FILTERS::Speedway::Params& params)
{
  m_lastSpeedwayParams = params;
}

void FilterStats::SetLastWaveParams(const FILTERS::Wave::Params& params)
{
  m_lastWaveParams = params;
}

void FilterStats::SetLastYOnlyParams(const FILTERS::YOnly::Params& params)
{
  m_lastYOnlyParams = params;
}

void FilterStats::SetLastGeneralSpeed(const float val)
{
  m_lastGeneralSpeed = val;
}

void FilterStats::SetLastPrevX(const uint32_t val)
{
  m_lastPrevX = val;
}

void FilterStats::SetLastPrevY(const uint32_t val)
{
  m_lastPrevY = val;
}

void FilterStats::SetLastTranBuffYLineStart(const uint32_t val)
{
  m_lastTranBuffYLineStart = val;
}

void FilterStats::SetLastTranDiffFactor(const int32_t val)
{
  m_lastTranDiffFactor = val;
}

} // namespace GOOM
