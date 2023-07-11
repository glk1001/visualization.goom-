#pragma once

#include "filter_buffer_striper.h"
#include "filter_buffers.h"
#include "filter_settings.h"
#include "utils/name_value_pairs.h"
#include "zoom_vector.h"

#include <cstdint>
#include <memory>
#include <span>
#include <string>

namespace GOOM
{
class PluginInfo;

namespace UTILS
{
class Parallel;
}

namespace FILTER_FX
{

class FilterBuffersService
{
  using FilterBuffers = ZoomFilterBuffers<ZoomFilterBufferStriper>;

public:
  FilterBuffersService(UTILS::Parallel& parallel,
                       const PluginInfo& goomInfo,
                       const NormalizedCoordsConverter& normalizedCoordsConverter,
                       std::unique_ptr<IZoomVector> zoomVector) noexcept;

  auto Start() noexcept -> void;

  [[nodiscard]] auto GetCurrentFilterEffectsSettings() const noexcept
      -> const FilterEffectsSettings&;
  auto SetFilterEffectsSettings(const FilterEffectsSettings& filterEffectsSettings) noexcept
      -> void;
  auto SetFilterTransformBufferSettings(
      const FilterTransformBufferSettings& filterTransformBufferSettings) noexcept -> void;
  [[nodiscard]] auto HaveFilterSettingsChanged() const noexcept -> bool;

  auto UpdateTransformBuffer() noexcept -> void;
  [[nodiscard]] auto IsTransformBufferReady() const noexcept -> bool;
  auto UpdateSrcePosFilterBuffer(float transformBufferLerpFactor,
                                 std_spn::span<Point2dFlt> srceFilterPosBuffer) const noexcept
      -> void;
  auto CopyTransformBuffer(std_spn::span<Point2dFlt>& destBuff) noexcept -> void;

  auto UpdateTransformBufferLerpData(
      const TransformBufferLerpData& transformBufferLerpData) noexcept -> void;
  [[nodiscard]] auto GetTransformBufferLerpFactor() const noexcept -> uint32_t;
  auto SetTransformBufferLerpFactor(uint32_t value) -> void;
  static constexpr auto MAX_TRAN_LERP_VALUE = 65536U;

  [[nodiscard]] auto GetNameValueParams(const std::string& paramGroup) const noexcept
      -> UTILS::NameValuePairs;

private:
  std::unique_ptr<IZoomVector> m_zoomVector;
  FilterBuffers m_filterBuffers;
  uint32_t m_transformBufferLerpFactor = 0U;
  Viewport m_nextFilterViewport{};
  bool m_pendingFilterViewport = false;

  FilterEffectsSettings m_currentFilterEffectsSettings{};
  FilterEffectsSettings m_nextFilterEffectsSettings{};
  bool m_pendingFilterEffectsSettings = false;

  auto UpdateFilterEffectsSettings() noexcept -> void;
  auto UpdateZoomVectorFilterEffectsSettings() noexcept -> void;
  [[nodiscard]] auto IsStartingFreshTransformBuffer() const noexcept -> bool;
  auto StartFreshTransformBuffer() noexcept -> void;
};

inline auto FilterBuffersService::IsTransformBufferReady() const noexcept -> bool
{
  return m_filterBuffers.IsTransformBufferReady();
}

inline auto FilterBuffersService::UpdateSrcePosFilterBuffer(
    const float transformBufferLerpFactor,
    std_spn::span<Point2dFlt> srceFilterPosBuffer) const noexcept -> void
{
  m_filterBuffers.UpdateSrcePosFilterBuffer(transformBufferLerpFactor, srceFilterPosBuffer);
}

inline auto FilterBuffersService::CopyTransformBuffer(std_spn::span<Point2dFlt>& destBuff) noexcept
    -> void
{
  m_filterBuffers.CopyTransformBuffer(destBuff);
}

inline auto FilterBuffersService::GetCurrentFilterEffectsSettings() const noexcept
    -> const FilterEffectsSettings&
{
  return m_currentFilterEffectsSettings;
}

inline auto FilterBuffersService::GetTransformBufferLerpFactor() const noexcept -> uint32_t
{
  return m_transformBufferLerpFactor;
}

inline auto FilterBuffersService::SetTransformBufferLerpFactor(const uint32_t value) -> void
{
  m_transformBufferLerpFactor = value;
}

inline auto FilterBuffersService::HaveFilterSettingsChanged() const noexcept -> bool
{
  return m_filterBuffers.HaveFilterSettingsChanged();
}

} // namespace FILTER_FX
} // namespace GOOM
