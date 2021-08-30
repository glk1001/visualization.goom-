#ifndef VISUALIZATION_GOOM_FILTER_BUFFERS_MANAGER_H
#define VISUALIZATION_GOOM_FILTER_BUFFERS_MANAGER_H

#include "filter_buffers.h"
#include "goom/filter_data.h"
#include "v2d.h"

#include <cstdint>

namespace GOOM
{

class PluginInfo;

namespace UTILS
{
class Parallel;
}; // namespace UTILS

namespace FILTERS
{

class IZoomVector;

class ZoomFilterBuffersManager
{
public:
  using NeighborhoodCoeffArray = ZoomFilterBuffers::NeighborhoodCoeffArray;
  using NeighborhoodPixelArray = ZoomFilterBuffers::NeighborhoodPixelArray;

  ZoomFilterBuffersManager(UTILS::Parallel& p,
                           const std::shared_ptr<const PluginInfo>& goomInfo,
                           IZoomVector& zoomVector,
                           FilterStats& stats) noexcept;

  void Start();

  void ChangeFilterSettings(const ZoomFilterData& filterSettings);

  void UpdateTranBuffers();
  void UpdateTranLerpFactor(int32_t switchIncr, float switchMult);

  using SourcePointInfo = ZoomFilterBuffers::SourcePointInfo;
  [[nodiscard]] auto GetSourcePointInfo(size_t buffPos) const -> SourcePointInfo;

  void UpdateLastStats() const;

private:
  bool m_started = false;
  IZoomVector& m_zoomVector;
  ZoomFilterBuffers m_filterBuffers;
  FilterStats& m_stats;

  ZoomFilterData m_currentFilterSettings{};
  ZoomFilterData m_nextFilterSettings{};
  bool m_pendingFilterSettings = false;

  void UpdateFilterBuffersSettings();
  void UpdateZoomVectorSettings();
  auto AreStartingFreshTranBuffers() const -> bool;
  void StartFreshTranBuffers();
};

inline auto ZoomFilterBuffersManager::GetSourcePointInfo(size_t buffPos) const -> SourcePointInfo
{
  return m_filterBuffers.GetSourcePointInfo(buffPos);
}

} // namespace FILTERS
} // namespace GOOM

#endif // VISUALIZATION_GOOM_FILTER_BUFFERS_MANAGER_H
