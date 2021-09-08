#ifndef VISUALIZATION_GOOM_FILTER_BUFFERS_SERVICE_H
#define VISUALIZATION_GOOM_FILTER_BUFFERS_SERVICE_H

#include "filter_buffers.h"
#include "filter_settings.h"
#include "v2d.h"

#include <cstdint>
#include <memory>

namespace GOOM
{

class PluginInfo;

namespace UTILS
{
class Parallel;
} // namespace UTILS

namespace FILTERS
{

class IZoomVector;

class FilterBuffersService
{
public:
  FilterBuffersService(UTILS::Parallel& parallel,
                       const std::shared_ptr<const PluginInfo>& goomInfo,
                       std::unique_ptr<IZoomVector> zoomVector) noexcept;

  void Start();

  void SetFilterBufferSettings(const ZoomFilterBufferSettings& filterBufferSettings);
  void SetFilterEffectsSettings(const ZoomFilterEffectsSettings& filterEffectsSettings);

  auto GetTranLerpFactor() const -> int32_t;

  void UpdateTranBuffers();
  void UpdateTranLerpFactor(int32_t tranLerpIncrement, float tranLerpToMaxSwitchMult);

  using SourcePointInfo = ZoomFilterBuffers::SourcePointInfo;
  [[nodiscard]] auto GetSourcePointInfo(size_t buffPos) const -> SourcePointInfo;

  [[nodiscard]] auto GetNameValueParams(const std::string& paramGroup) const
      -> std::vector<std::pair<std::string, std::string>>;

private:
  std::unique_ptr<IZoomVector> m_zoomVector;
  ZoomFilterBuffers m_filterBuffers;

  ZoomFilterEffectsSettings m_currentFilterEffectsSettings{};
  ZoomFilterEffectsSettings m_nextFilterEffectsSettings{};
  bool m_pendingFilterEffectsSettings = false;

  void UpdateFilterEffectsSettings();
  auto AreStartingFreshTranBuffers() const -> bool;
  void StartFreshTranBuffers();
};

inline auto FilterBuffersService::GetSourcePointInfo(const size_t buffPos) const -> SourcePointInfo
{
  return m_filterBuffers.GetSourcePointInfo(buffPos);
}

inline auto FilterBuffersService::GetTranLerpFactor() const -> int32_t
{
  return m_filterBuffers.GetTranLerpFactor();
}

} // namespace FILTERS
} // namespace GOOM

#endif // VISUALIZATION_GOOM_FILTER_BUFFERS_SERVICE_H
