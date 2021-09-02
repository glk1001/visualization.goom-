#ifndef VISUALIZATION_GOOM_FILTER_BUFFERS_SERVICE_H
#define VISUALIZATION_GOOM_FILTER_BUFFERS_SERVICE_H

#include "filter_buffers.h"
#include "goom/filter_data.h"
#include "v2d.h"

#include <cstdint>
#include <memory>

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
class SpeedCoefficientsEffect;

class ZoomFilterBuffersService
{
public:
  ZoomFilterBuffersService(UTILS::Parallel& p,
                           const std::shared_ptr<const PluginInfo>& goomInfo,
                           std::unique_ptr<IZoomVector> zoomVector) noexcept;

  void Start();

  void SetFilterSettings(const ZoomFilterData& filterSettings);
  void SetSpeedCoefficientsEffect(std::shared_ptr<SpeedCoefficientsEffect> val);
  void UpdatePlaneEffects();

  void UpdateTranBuffers();
  void UpdateTranLerpFactor(int32_t switchIncr, float switchMult);

  using SourcePointInfo = ZoomFilterBuffers::SourcePointInfo;
  [[nodiscard]] auto GetSourcePointInfo(size_t buffPos) const -> SourcePointInfo;

private:
  const uint32_t m_screenWidth;
  std::unique_ptr<IZoomVector> m_zoomVector;
  ZoomFilterBuffers m_filterBuffers;

  ZoomFilterData m_currentFilterSettings{};
  ZoomFilterData m_nextFilterSettings{};
  std::shared_ptr<SpeedCoefficientsEffect> m_nextSpeedCoefficientsEffect{};
  bool m_pendingPlaneEffects = false;
  bool m_pendingFilterSettings = false;

  void UpdateFilterSettings();
  auto AreStartingFreshTranBuffers() const -> bool;
  void StartFreshTranBuffers();
};

inline auto ZoomFilterBuffersService::GetSourcePointInfo(const size_t buffPos) const
    -> SourcePointInfo
{
  return m_filterBuffers.GetSourcePointInfo(buffPos);
}

} // namespace FILTERS
} // namespace GOOM

#endif // VISUALIZATION_GOOM_FILTER_BUFFERS_SERVICE_H
