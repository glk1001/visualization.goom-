#pragma once

#include "filter_buffers.h"
#include "filter_settings.h"
#include "utils/name_value_pairs.h"
#include "zoom_vector.h"

#include <cstdint>
#include <memory>

namespace GOOM
{

class PluginInfo;

namespace UTILS
{
class Parallel;
}

namespace VISUAL_FX::FILTERS
{

class FilterBuffersService
{
public:
  FilterBuffersService(UTILS::Parallel& parallel,
                       const PluginInfo& goomInfo,
                       const NormalizedCoordsConverter& normalizedCoordsConverter,
                       std::unique_ptr<IZoomVector> zoomVector) noexcept;

  void Start();

  [[nodiscard]] auto GetCurrentFilterEffectsSettings() const -> const ZoomFilterEffectsSettings&;
  void SetFilterEffectsSettings(const ZoomFilterEffectsSettings& filterEffectsSettings);
  void SetFilterBufferSettings(const ZoomFilterBufferSettings& filterBufferSettings);

  [[nodiscard]] auto GetTranLerpFactor() const -> int32_t;

  void UpdateTranBuffers();
  void UpdateTranLerpFactor(int32_t tranLerpIncrement, float tranLerpToMaxSwitchMult);

  using SourcePointInfo = ZoomFilterBuffers::SourcePointInfo;
  [[nodiscard]] auto GetSourcePointInfo(size_t buffPos) const -> SourcePointInfo;

  [[nodiscard]] auto GetNameValueParams(const std::string& paramGroup) const
      -> UTILS::NameValuePairs;

private:
  std::unique_ptr<IZoomVector> m_zoomVector;
  ZoomFilterBuffers m_filterBuffers;

  ZoomFilterEffectsSettings m_currentFilterEffectsSettings{};
  ZoomFilterEffectsSettings m_nextFilterEffectsSettings{};
  bool m_pendingFilterEffectsSettings = false;

  void UpdateFilterEffectsSettings();
  void UpdateZoomVectorFilterEffectsSettings();
  [[nodiscard]] auto AreStartingFreshTranBuffers() const -> bool;
  void StartFreshTranBuffers();
};

inline auto FilterBuffersService::GetCurrentFilterEffectsSettings() const
    -> const ZoomFilterEffectsSettings&
{
  return m_currentFilterEffectsSettings;
}

inline auto FilterBuffersService::GetSourcePointInfo(const size_t buffPos) const -> SourcePointInfo
{
  return m_filterBuffers.GetSourcePointInfo(buffPos);
}

inline auto FilterBuffersService::GetTranLerpFactor() const -> int32_t
{
  return m_filterBuffers.GetTranLerpFactor();
}

} // namespace VISUAL_FX::FILTERS
} // namespace GOOM
