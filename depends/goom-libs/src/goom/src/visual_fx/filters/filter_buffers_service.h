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

  void Start() noexcept;

  [[nodiscard]] auto GetCurrentFilterEffectsSettings() const noexcept
      -> const ZoomFilterEffectsSettings&;
  void SetFilterEffectsSettings(const ZoomFilterEffectsSettings& filterEffectsSettings) noexcept;
  void SetFilterBufferSettings(const ZoomFilterBufferSettings& filterBufferSettings) noexcept;

  [[nodiscard]] auto GetTranLerpFactor() const noexcept -> int32_t;

  void UpdateTranBuffers() noexcept;
  void UpdateTranLerpFactor(int32_t tranLerpIncrement, float tranLerpToMaxSwitchMult) noexcept;

  using SourcePointInfo = ZoomFilterBuffers::SourcePointInfo;
  [[nodiscard]] auto GetSourcePointInfo(size_t buffPos) const noexcept -> SourcePointInfo;

  [[nodiscard]] auto GetNameValueParams(const std::string& paramGroup) const noexcept
      -> UTILS::NameValuePairs;

private:
  std::unique_ptr<IZoomVector> m_zoomVector;
  ZoomFilterBuffers m_filterBuffers;

  ZoomFilterEffectsSettings m_currentFilterEffectsSettings{};
  ZoomFilterEffectsSettings m_nextFilterEffectsSettings{};
  bool m_pendingFilterEffectsSettings = false;

  void UpdateFilterEffectsSettings() noexcept;
  void UpdateZoomVectorFilterEffectsSettings() noexcept;
  [[nodiscard]] auto AreStartingFreshTranBuffers() const noexcept -> bool;
  void StartFreshTranBuffers() noexcept;
};

inline auto FilterBuffersService::GetCurrentFilterEffectsSettings() const noexcept
    -> const ZoomFilterEffectsSettings&
{
  return m_currentFilterEffectsSettings;
}

inline auto FilterBuffersService::GetSourcePointInfo(const size_t buffPos) const noexcept
    -> SourcePointInfo
{
  return m_filterBuffers.GetSourcePointInfo(buffPos);
}

inline auto FilterBuffersService::GetTranLerpFactor() const noexcept -> int32_t
{
  return m_filterBuffers.GetTranLerpFactor();
}

} // namespace VISUAL_FX::FILTERS
} // namespace GOOM
