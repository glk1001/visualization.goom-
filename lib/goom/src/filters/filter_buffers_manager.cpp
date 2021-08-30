#include "filter_buffers_manager.h"

#include "../stats/filter_stats.h"
#include "filter_buffers.h"
#include "filter_data.h"
#include "filter_normalized_coords.h"
#include "goom_plugin_info.h"
#include "goom_zoom_vector.h"
#include "goomutils/goomrand.h"
#include "goomutils/logging_control.h"
//#undef NO_LOGGING
#include "goomutils/logging.h"
#include "goomutils/mathutils.h"
#include "goomutils/parallel_utils.h"
#undef NDEBUG
#include <cassert>
#include <cstdint>
#include <string>

#if __cplusplus <= 201402L
namespace GOOM
{
namespace FILTERS
{
#else
namespace GOOM::FILTERS
{
#endif


using FILTERS::IZoomVector;
using FILTERS::NormalizedCoords;
using FILTERS::ZoomFilterBuffers;
using UTILS::floats_equal;
using UTILS::GetRandInRange;
using UTILS::Logging;
using UTILS::Parallel;

constexpr float MAX_MAX_SPEED_COEFF = +4.01F;

ZoomFilterBuffersManager::ZoomFilterBuffersManager(
    Parallel& p,
    const std::shared_ptr<const PluginInfo>& goomInfo,
    IZoomVector& zoomVector) noexcept
  : m_zoomVector{zoomVector},
    m_filterBuffers{p, goomInfo, [this](const NormalizedCoords& normalizedCoords) {
                      return m_zoomVector.GetZoomPoint(normalizedCoords);
                    }}
{
}

void ZoomFilterBuffersManager::SetStats(const std::shared_ptr<FilterStats> stats)
{
  m_stats = stats;
  m_filterBuffers.SetStats(stats);
}

void ZoomFilterBuffersManager::Start()
{
  assert(m_stats != nullptr);

  m_started = true;

  ChangeFilterSettings(m_currentFilterSettings);

  m_zoomVector.SetFilterSettings(m_currentFilterSettings);
  m_zoomVector.SetFilterStats(*m_stats);

  UpdateFilterBuffersSettings();
  m_filterBuffers.Start();
}

void ZoomFilterBuffersManager::UpdateLastStats() const
{
  m_zoomVector.UpdateLastStats();

  m_stats->SetLastJustChangedFilterSettings(m_pendingFilterSettings);
  m_stats->SetLastTranBuffYLineStart(m_filterBuffers.GetTranBuffYLineStart());
  m_stats->SetLastTranDiffFactor(m_filterBuffers.GetTranLerpFactor());
}

void ZoomFilterBuffersManager::ChangeFilterSettings(const ZoomFilterData& filterSettings)
{
  assert(m_started);

  m_stats->DoChangeFilterSettings(filterSettings);

  m_nextFilterSettings = filterSettings;
  m_pendingFilterSettings = true;
}

void ZoomFilterBuffersManager::UpdateTranBuffers()
{
  m_stats->UpdateTranBuffersStart();

  m_filterBuffers.UpdateTranBuffers();

  if (AreStartingFreshTranBuffers())
  {
    StartFreshTranBuffers();
  }

  m_stats->UpdateTranBuffersEnd(m_currentFilterSettings.mode,
                                m_filterBuffers.GetTranBuffersState());
}

inline auto ZoomFilterBuffersManager::AreStartingFreshTranBuffers() const -> bool
{
  return m_filterBuffers.GetTranBuffersState() ==
         ZoomFilterBuffers::TranBuffersState::START_FRESH_TRAN_BUFFERS;
}

void ZoomFilterBuffersManager::StartFreshTranBuffers()
{
  // Don't start making new stripes until filter settings change.
  if (!m_pendingFilterSettings)
  {
    return;
  }

  m_pendingFilterSettings = false;
  m_currentFilterSettings = m_nextFilterSettings;

  UpdateZoomVectorSettings();
  UpdateFilterBuffersSettings();
}

inline void ZoomFilterBuffersManager::UpdateZoomVectorSettings()
{
  m_zoomVector.SetFilterSettings(m_currentFilterSettings);
  // TODO Random calc should not be here
  m_zoomVector.SetMaxSpeedCoeff(GetRandInRange(0.5F, 1.0F) * MAX_MAX_SPEED_COEFF);
}

inline void ZoomFilterBuffersManager::UpdateFilterBuffersSettings()
{
  m_filterBuffers.SetBuffMidPoint(m_currentFilterSettings.zoomMidPoint);
  m_filterBuffers.NotifyFilterSettingsHaveChanged();
}

void ZoomFilterBuffersManager::UpdateTranLerpFactor(const int32_t switchIncr,
                                                    const float switchMult)
{
  int32_t tranLerpFactor = m_filterBuffers.GetTranLerpFactor();

  if (switchIncr != 0)
  {
    m_stats->DoSwitchIncrNotZero();
    tranLerpFactor =
        stdnew::clamp(tranLerpFactor + switchIncr, 0, ZoomFilterBuffers::GetMaxTranLerpFactor());
  }

  if (!floats_equal(switchMult, 1.0F))
  {
    m_stats->DoSwitchMultNotOne();

    tranLerpFactor = static_cast<int32_t>(
        stdnew::lerp(static_cast<float>(ZoomFilterBuffers::GetMaxTranLerpFactor()),
                     static_cast<float>(tranLerpFactor), switchMult));
  }

  m_filterBuffers.SetTranLerpFactor(tranLerpFactor);
}

#if __cplusplus <= 201402L
} // namespace FILTERS
} // namespace GOOM
#else
} // namespace GOOM::FILTERS
#endif
