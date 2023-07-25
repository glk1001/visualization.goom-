//#undef NO_LOGGING

#include "filter_buffers_service.h"

#include "filter_buffer_striper.h"
#include "filter_buffers.h"
#include "filter_settings.h"
#include "goom_config.h"
#include "goom_logger.h"
#include "goom_plugin_info.h"
#include "normalized_coords.h"
#include "utils/math/misc.h"
#include "utils/parallel_utils.h"
#include "zoom_vector.h"

namespace GOOM::FILTER_FX
{

using UTILS::GetPair;
using UTILS::MoveNameValuePairs;
using UTILS::NameValuePairs;
using UTILS::Parallel;

FilterBuffersService::FilterBuffersService(
    Parallel& parallel,
    const PluginInfo& goomInfo,
    const NormalizedCoordsConverter& normalizedCoordsConverter,
    std::unique_ptr<IZoomVector> zoomVector) noexcept
  : m_zoomVector{std::move(zoomVector)},
    m_filterBuffers{std::make_unique<ZoomFilterBufferStriper>(
        parallel,
        goomInfo,
        normalizedCoordsConverter,
        [this](const NormalizedCoords& normalizedCoords,
               const NormalizedCoords& normalizedFilterViewportCoords)
        { return m_zoomVector->GetZoomInPoint(normalizedCoords, normalizedFilterViewportCoords); })}
{
}

auto FilterBuffersService::SetFilterEffectsSettings(
    const FilterEffectsSettings& filterEffectsSettings) noexcept -> void
{
  m_nextFilterEffectsSettings    = filterEffectsSettings;
  m_pendingFilterEffectsSettings = true;
}

auto FilterBuffersService::SetFilterTransformBufferSettings(
    const FilterTransformBufferSettings& filterTransformBufferSettings) noexcept -> void
{
  UpdateTransformBufferViewport(filterTransformBufferSettings.viewport);

  UpdateTransformBufferLerpData({filterTransformBufferSettings.lerpData.lerpIncrement,
                                 filterTransformBufferSettings.lerpData.lerpToMaxLerp});
}

auto FilterBuffersService::Start() noexcept -> void
{
  Expects(m_pendingFilterViewport);
  Expects(m_pendingFilterEffectsSettings);
  Expects(m_nextFilterEffectsSettings.zoomInCoefficientsEffect != nullptr);

  StartFreshTransformBuffer();

  m_filterBuffers.Start();
}

auto FilterBuffersService::StartFreshTransformBuffer() noexcept -> void
{
  if (m_pendingFilterEffectsSettings)
  {
    m_nextFilterEffectsSettings.afterEffectsSettings.rotationAdjustments.Reset();
    m_zoomVector->SetFilterEffectsSettings(m_nextFilterEffectsSettings);
    m_filterBuffers.SetTransformBufferMidpoint(m_nextFilterEffectsSettings.zoomMidpoint);
    m_pendingFilterEffectsSettings = false;
    // Notify buffer maker of settings change - now we'll get a fresh buffer.
    m_filterBuffers.NotifyFilterSettingsHaveChanged();
  }

  if (m_pendingFilterViewport)
  {
    m_filterBuffers.SetFilterViewport(m_nextFilterViewport);
    m_pendingFilterViewport = false;
    // Notify buffer maker of settings change - now we'll get a fresh buffer.
    m_filterBuffers.NotifyFilterSettingsHaveChanged();
  }
}

auto FilterBuffersService::UpdateTransformBuffer() noexcept -> void
{
  m_filterBuffers.UpdateTransformBuffer();

  if (m_filterBuffers.GetTransformBufferState() ==
      FilterBuffers::TransformBufferState::START_FRESH_TRANSFORM_BUFFER)
  {
    StartFreshTransformBuffer();
  }
}

inline auto FilterBuffersService::UpdateTransformBufferViewport(const Viewport& viewport) noexcept
    -> void
{
  if (m_nextFilterViewport == viewport)
  {
    return;
  }

  m_nextFilterViewport    = viewport;
  m_pendingFilterViewport = true;
}

inline auto FilterBuffersService::UpdateTransformBufferLerpData(
    const TransformBufferLerpData& transformBufferLerpData) noexcept -> void
{
  if (transformBufferLerpData.lerpIncrement > 0.0F)
  {
    m_transformBufferLerpFactor =
        std::min(m_transformBufferLerpFactor + transformBufferLerpData.lerpIncrement, 1.0F);
  }

  if (transformBufferLerpData.lerpToMaxLerp > 0.0F)
  {
    m_transformBufferLerpFactor =
        STD20::lerp(m_transformBufferLerpFactor, 1.0F, transformBufferLerpData.lerpToMaxLerp);
  }
}

auto FilterBuffersService::GetNameValueParams(const std::string& paramGroup) const noexcept
    -> NameValuePairs
{
  auto nameValuePairs = NameValuePairs{};

  nameValuePairs.emplace_back(GetPair(paramGroup, "lerpFactor", m_transformBufferLerpFactor));
  MoveNameValuePairs(m_zoomVector->GetNameValueParams(paramGroup), nameValuePairs);

  return nameValuePairs;
}

} // namespace GOOM::FILTER_FX
