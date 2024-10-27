module;

//#undef NO_LOGGING

#include <cmath>
#include <cstdint>
#include <format>
#include <memory>
#include <string>
#include <thread>
#include <utility>

module Goom.FilterFx.FilterBuffersService;

import Goom.FilterFx.FilterBuffers;
import Goom.FilterFx.FilterSettings;
import Goom.FilterFx.NormalizedCoords;
import Goom.FilterFx.ZoomVector;
import Goom.Utils.GoomTime;
import Goom.Utils.NameValuePairs;
import Goom.Lib.AssertUtils;
import Goom.PluginInfo;

namespace GOOM::FILTER_FX
{

using UTILS::GetPair;
using UTILS::NameValuePairs;

FilterBuffersService::FilterBuffersService(
    const PluginInfo& goomInfo,
    const NormalizedCoordsConverter& normalizedCoordsConverter,
    std::unique_ptr<IZoomVector> zoomVector) noexcept
  : m_goomTime{&goomInfo.GetTime()},
    m_zoomVector{std::move(zoomVector)},
    m_filterBuffers{goomInfo,
                    normalizedCoordsConverter,
                    [this](const NormalizedCoords& normalizedCoords)
                    { return m_zoomVector->GetZoomPoint(normalizedCoords); }}
{
}

auto FilterBuffersService::SetFilterEffectsSettings(
    const FilterEffectsSettings& filterEffectsSettings) noexcept -> void
{
  m_nextFilterEffectsSettings    = filterEffectsSettings;
  m_pendingFilterEffectsSettings = true;
}

auto FilterBuffersService::Start() noexcept -> void
{
  Expects(m_pendingFilterEffectsSettings);
  Expects(m_nextFilterEffectsSettings.zoomAdjustmentEffect != nullptr);

  UpdateAllPendingSettings();

  m_numPendingFilterEffectsChanges   = 0U;
  m_totalGoomTimeOfBufferProcessing  = 0U;
  m_totalGoomTimeBetweenBufferResets = 0U;
  m_numFilterBuffersCompleted        = 0U;
  m_numFilterBufferResets            = 0U;

  m_filterBuffers.Start();

  StartFilterBufferThread();
}

auto FilterBuffersService::Finish() noexcept -> void
{
  m_filterBuffers.Finish();
  m_bufferProducerThread.join();
}

auto FilterBuffersService::StartFilterBufferThread() noexcept -> void
{
  m_bufferProducerThread = std::thread{&ZoomFilterBuffers::FilterBufferThread, &m_filterBuffers};
}

auto FilterBuffersService::UpdateAllPendingSettings() noexcept -> void
{
  m_nextFilterEffectsSettings.afterEffectsSettings.rotationAdjustments.Reset();
  m_zoomVector->SetFilterEffectsSettings(m_nextFilterEffectsSettings);
  m_filterBuffers.SetFilterBufferMidpoint(m_nextFilterEffectsSettings.zoomMidpoint);
  m_pendingFilterEffectsSettings = false;
}

auto FilterBuffersService::UpdateFilterBuffer() noexcept -> void
{
  if (ZoomFilterBuffers::UpdateStatus::HAS_BEEN_COPIED == m_filterBuffers.GetUpdateStatus())
  {
    UpdateCompletedFilterBufferStats();
  }

  if (m_pendingFilterEffectsSettings and
      (ZoomFilterBuffers::UpdateStatus::HAS_BEEN_COPIED == m_filterBuffers.GetUpdateStatus()))
  {
    CompletePendingSettings();
  }
}

auto FilterBuffersService::CompletePendingSettings() noexcept -> void
{
  m_filterBuffers.ResetFilterBufferToStart();
  ++m_numFilterBufferResets;
  m_totalGoomTimeBetweenBufferResets +=
      m_goomTime->GetElapsedTimeSince(m_goomTimeAtFilterBufferReset);

  UpdateAllPendingSettings();
  Ensures(not m_pendingFilterEffectsSettings);
  ++m_numPendingFilterEffectsChanges;

  m_filterBuffers.StartFilterBufferUpdates();
  m_goomTimeAtFilterBufferStart = m_goomTime->GetCurrentTime();
  m_goomTimeAtFilterBufferReset = m_goomTime->GetCurrentTime();
}

auto FilterBuffersService::UpdateCompletedFilterBufferStats() noexcept -> void
{
  if (0U == m_goomTimeAtFilterBufferStart)
  {
    return;
  }

  m_totalGoomTimeOfBufferProcessing +=
      m_goomTime->GetElapsedTimeSince(m_goomTimeAtFilterBufferStart);
  ++m_numFilterBuffersCompleted;

  m_goomTimeAtFilterBufferStart = 0U;
}

auto FilterBuffersService::GetNameValueParams() const noexcept -> NameValuePairs
{
  static constexpr auto* PARAM_GROUP = "Buffer Service";

  return {GetPair(PARAM_GROUP,
                  "params",
                  std::format("{}, {}, {}, {}",
                              m_numPendingFilterEffectsChanges,
                              m_numFilterBuffersCompleted,
                              GetAverageGoomTimeOfBufferProcessing(),
                              GetAverageGoomTimeBetweenBufferResets()))};
}

auto FilterBuffersService::GetZoomVectorNameValueParams() const noexcept -> NameValuePairs
{
  return m_zoomVector->GetNameValueParams();
}

auto FilterBuffersService::GetAfterEffectsNameValueParams() const noexcept -> NameValuePairs
{
  return m_zoomVector->GetAfterEffectsNameValueParams();
}

auto FilterBuffersService::GetAverageGoomTimeOfBufferProcessing() const noexcept -> uint32_t
{
  if (0U == m_numFilterBuffersCompleted)
  {
    return 0U;
  }

  return static_cast<uint32_t>(std::round(static_cast<float>(m_totalGoomTimeOfBufferProcessing) /
                                          static_cast<float>(m_numFilterBuffersCompleted)));
}

auto FilterBuffersService::GetAverageGoomTimeBetweenBufferResets() const noexcept -> uint32_t
{
  if (0U == m_numFilterBuffersCompleted)
  {
    return 0U;
  }

  return static_cast<uint32_t>(std::round(static_cast<float>(m_totalGoomTimeBetweenBufferResets) /
                                          static_cast<float>(m_numFilterBufferResets)));
}

} // namespace GOOM::FILTER_FX
