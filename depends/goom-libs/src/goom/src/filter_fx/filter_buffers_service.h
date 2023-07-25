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
} // namespace GOOM

namespace GOOM::FILTER_FX
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
  // TODO - In tests only
  [[nodiscard]] auto HaveFilterSettingsChanged() const noexcept -> bool;

  auto UpdateTransformBuffer() noexcept -> void;
  [[nodiscard]] auto IsTransformBufferReady() const noexcept -> bool;
  [[nodiscard]] auto GetPreviousTransformBuffer() const noexcept -> const std::vector<Point2dFlt>&;
  auto CopyTransformBuffer(std_spn::span<Point2dFlt>& destBuff) noexcept -> void;
  auto RestartTransformBuffer() noexcept -> void;

  [[nodiscard]] auto GetNameValueParams(const std::string& paramGroup) const noexcept
      -> UTILS::NameValuePairs;

private:
  std::unique_ptr<IZoomVector> m_zoomVector;
  FilterBuffers m_filterBuffers;

  FilterEffectsSettings m_nextFilterEffectsSettings{};
  bool m_pendingFilterEffectsSettings = false;

  auto StartFreshTransformBuffer() noexcept -> void;
};

inline auto FilterBuffersService::IsTransformBufferReady() const noexcept -> bool
{
  return m_filterBuffers.IsTransformBufferReady();
}

inline auto FilterBuffersService::GetPreviousTransformBuffer() const noexcept
    -> const std::vector<Point2dFlt>&
{
  return m_filterBuffers.GetPreviousTransformBuffer();
}

inline auto FilterBuffersService::CopyTransformBuffer(std_spn::span<Point2dFlt>& destBuff) noexcept
    -> void
{
  m_filterBuffers.CopyTransformBuffer(destBuff);
}

inline auto FilterBuffersService::RestartTransformBuffer() noexcept -> void
{
  m_filterBuffers.RestartTransformBuffer();
}

inline auto FilterBuffersService::HaveFilterSettingsChanged() const noexcept -> bool
{
  return m_filterBuffers.HaveFilterSettingsChanged();
}

} // namespace GOOM::FILTER_FX
