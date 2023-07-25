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

  [[nodiscard]] auto GetTransformBufferLerpFactor() const noexcept -> float;
  auto SetTransformBufferLerpFactor(float value) -> void;

  [[nodiscard]] auto GetNameValueParams(const std::string& paramGroup) const noexcept
      -> UTILS::NameValuePairs;

private:
  std::unique_ptr<IZoomVector> m_zoomVector;
  FilterBuffers m_filterBuffers;
  float m_transformBufferLerpFactor = 0.0F;

  Viewport m_nextFilterViewport{};
  bool m_pendingFilterViewport = true;
  FilterEffectsSettings m_nextFilterEffectsSettings{};
  bool m_pendingFilterEffectsSettings = false;

  auto UpdateTransformBufferViewport(const Viewport& viewport) noexcept -> void;
  auto UpdateTransformBufferLerpData(
      const TransformBufferLerpData& transformBufferLerpData) noexcept -> void;

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

inline auto FilterBuffersService::GetTransformBufferLerpFactor() const noexcept -> float
{
  return m_transformBufferLerpFactor;
}

inline auto FilterBuffersService::SetTransformBufferLerpFactor(const float value) -> void
{
  Expects(value >= 0.0F);
  Expects(value <= 1.0F);
  m_transformBufferLerpFactor = value;
}

inline auto FilterBuffersService::HaveFilterSettingsChanged() const noexcept -> bool
{
  return m_filterBuffers.HaveFilterSettingsChanged();
}

} // namespace FILTER_FX
} // namespace GOOM
