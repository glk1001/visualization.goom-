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
using UTILS::MATH::FloatsEqual;

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

auto FilterBuffersService::SetFilterTransformBufferSettings(
    const FilterTransformBufferSettings& filterTransformBufferSettings) noexcept -> void
{
  m_nextFilterViewport    = filterTransformBufferSettings.viewport;
  m_pendingFilterViewport = true;

  UpdateTransformBufferLerpData({filterTransformBufferSettings.lerpData.lerpIncrement,
                                 filterTransformBufferSettings.lerpData.lerpToMaxLerp});
}

auto FilterBuffersService::SetFilterEffectsSettings(
    const FilterEffectsSettings& filterEffectsSettings) noexcept -> void
{
  m_nextFilterEffectsSettings    = filterEffectsSettings;
  m_pendingFilterEffectsSettings = true;
}

auto FilterBuffersService::GetNameValueParams(const std::string& paramGroup) const noexcept
    -> NameValuePairs
{
  auto nameValuePairs = NameValuePairs{};

  nameValuePairs.emplace_back(GetPair(paramGroup, "lerpFactor", m_transformBufferLerpFactor));
  MoveNameValuePairs(m_zoomVector->GetNameValueParams(paramGroup), nameValuePairs);

  return nameValuePairs;
}

auto FilterBuffersService::Start() noexcept -> void
{
  m_currentFilterEffectsSettings = m_nextFilterEffectsSettings;
  Expects(m_currentFilterEffectsSettings.zoomInCoefficientsEffect != nullptr);

  m_nextFilterViewport    = Viewport{};
  m_pendingFilterViewport = true;

  UpdateFilterEffectsSettings();

  m_filterBuffers.Start();
}

inline auto FilterBuffersService::UpdateFilterEffectsSettings() noexcept -> void
{
  UpdateZoomVectorFilterEffectsSettings();

  if (m_pendingFilterViewport)
  {
    m_filterBuffers.SetFilterViewport(m_nextFilterViewport);
    m_pendingFilterViewport = false;
  }

  m_filterBuffers.SetTransformBufferMidpoint(m_currentFilterEffectsSettings.zoomMidpoint);
  m_filterBuffers.NotifyFilterSettingsHaveChanged();
}

inline auto FilterBuffersService::UpdateZoomVectorFilterEffectsSettings() noexcept -> void
{
  m_zoomVector->SetFilterEffectsSettings(m_currentFilterEffectsSettings);

  m_currentFilterEffectsSettings.afterEffectsSettings.rotationAdjustments.Reset();
}

auto FilterBuffersService::UpdateTransformBuffer() noexcept -> void
{
  m_filterBuffers.UpdateTransformBuffer();

  if (IsStartingFreshTransformBuffer())
  {
    StartFreshTransformBuffer();
  }
}

inline auto FilterBuffersService::IsStartingFreshTransformBuffer() const noexcept -> bool
{
  return m_filterBuffers.GetTransformBufferState() ==
         FilterBuffers::TransformBufferState::START_FRESH_TRANSFORM_BUFFER;
}

auto FilterBuffersService::StartFreshTransformBuffer() noexcept -> void
{
  // Don't start making new stripes until filter settings change.
  if (not m_pendingFilterEffectsSettings)
  {
    return;
  }

  m_currentFilterEffectsSettings = m_nextFilterEffectsSettings;

  UpdateFilterEffectsSettings();

  m_pendingFilterEffectsSettings = false;
}

inline auto FilterBuffersService::UpdateTransformBufferLerpData(
    const TransformBufferLerpData& transformBufferLerpData) noexcept -> void
{
  if (transformBufferLerpData.lerpIncrement != 0U)
  {
    m_transformBufferLerpFactor = std::min(
        m_transformBufferLerpFactor + transformBufferLerpData.lerpIncrement, MAX_TRAN_LERP_VALUE);
  }

  if (not FloatsEqual(transformBufferLerpData.lerpToMaxLerp, 1.0F))
  {
    m_transformBufferLerpFactor = STD20::lerp(
        MAX_TRAN_LERP_VALUE, m_transformBufferLerpFactor, transformBufferLerpData.lerpToMaxLerp);
  }
}

} // namespace GOOM::FILTER_FX
