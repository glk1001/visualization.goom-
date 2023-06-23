#undef NO_LOGGING

#include "filter_buffers_service.h"

#include "filter_buffer_striper.h"
#include "filter_buffers.h"
#include "filter_settings.h"
#include "goom_config.h"
#include "goom_logger.h"
#include "goom_plugin_info.h"
#include "normalized_coords.h"
#include "utils/debugging_logger.h"
#include "utils/math/misc.h"
#include "utils/parallel_utils.h"
#include "zoom_in_coefficients_effect.h"
#include "zoom_vector.h"

namespace GOOM::FILTER_FX
{

using UTILS::NameValuePairs;
using UTILS::Parallel;
using UTILS::MATH::FloatsEqual;

FilterBuffersService::FilterBuffersService(
    Parallel& parallel,
    const PluginInfo& goomInfo,
    const NormalizedCoordsConverter& normalizedCoordsConverter,
    std::unique_ptr<IZoomVector> zoomVector) noexcept
  : m_zoomVector{std::move(zoomVector)},
    m_filterBuffers{goomInfo,
                    std::make_unique<ZoomFilterBufferStriper>(
                        parallel,
                        goomInfo,
                        normalizedCoordsConverter,
                        [this](const NormalizedCoords& normalizedCoords,
                               const NormalizedCoords& normalizedFilterViewportCoords) {
                          return m_zoomVector->GetZoomInPoint(normalizedCoords,
                                                              normalizedFilterViewportCoords);
                        })}
{
}

auto FilterBuffersService::SetFilterBufferSettings(
    const ZoomFilterBufferSettings& filterBufferSettings) noexcept -> void
{
  m_filterBuffers.SetFilterViewport(filterBufferSettings.filterEffectViewport);

  UpdateTranLerpProperties(
      {filterBufferSettings.tranLerpIncrement, filterBufferSettings.tranLerpToMaxSwitchMult});
}

auto FilterBuffersService::SetFilterEffectsSettings(
    const ZoomFilterEffectsSettings& filterEffectsSettings) noexcept -> void
{
  m_nextFilterEffectsSettings    = filterEffectsSettings;
  m_pendingFilterEffectsSettings = true;
}

auto FilterBuffersService::GetNameValueParams(const std::string& paramGroup) const noexcept
    -> NameValuePairs
{
  return m_zoomVector->GetNameValueParams(paramGroup);
}

auto FilterBuffersService::Start() noexcept -> void
{
  m_currentFilterEffectsSettings = m_nextFilterEffectsSettings;
  Expects(m_currentFilterEffectsSettings.zoomInCoefficientsEffect != nullptr);

  UpdateFilterEffectsSettings();

  m_filterBuffers.Start();
}

inline auto FilterBuffersService::UpdateFilterEffectsSettings() noexcept -> void
{
  UpdateZoomVectorFilterEffectsSettings();

  m_filterBuffers.SetBuffMidpoint(m_currentFilterEffectsSettings.zoomMidpoint);
  m_filterBuffers.NotifyFilterSettingsHaveChanged();
}

inline auto FilterBuffersService::UpdateZoomVectorFilterEffectsSettings() noexcept -> void
{
  m_zoomVector->SetFilterSettings(m_currentFilterEffectsSettings);

  m_currentFilterEffectsSettings.afterEffectsSettings.rotationAdjustments.Reset();
}

auto FilterBuffersService::UpdateTranBuffers() noexcept -> void
{
  LogInfo(UTILS::GetGoomLogger(), "Starting update tran buffers.");

  m_filterBuffers.UpdateTranBuffers();

  m_filterPosDataReady =
      m_filterBuffers.GetTranBuffersState() == FilterBuffers::TranBuffersState::RESET_TRAN_BUFFERS;

  if (AreStartingFreshTranBuffers())
  {
    StartFreshTranBuffers();
  }
}

auto FilterBuffersService::IsFilterPosDataReady() const noexcept -> bool
{
  return m_filterPosDataReady;
}

inline auto FilterBuffersService::AreStartingFreshTranBuffers() const noexcept -> bool
{
  return m_filterBuffers.GetTranBuffersState() ==
         FilterBuffers::TranBuffersState::START_FRESH_TRAN_BUFFERS;
}

auto FilterBuffersService::StartFreshTranBuffers() noexcept -> void
{
  // Don't start making new stripes until filter settings change.
  if (!m_pendingFilterEffectsSettings)
  {
    return;
  }

  m_currentFilterEffectsSettings = m_nextFilterEffectsSettings;

  UpdateFilterEffectsSettings();

  m_pendingFilterEffectsSettings = false;
}

inline auto FilterBuffersService::UpdateTranLerpProperties(
    const TranLerpProperties& tranLerpProperties) noexcept -> void
{
  auto tranLerpFactor = m_filterBuffers.GetTranLerpFactor();

  if (tranLerpProperties.tranLerpIncrement != 0U)
  {
    tranLerpFactor = std::min(tranLerpFactor + tranLerpProperties.tranLerpIncrement,
                              FilterBuffers::GetMaxTranLerpFactor());
  }

  if (not FloatsEqual(tranLerpProperties.tranLerpToMaxSwitchMult, 1.0F))
  {
    tranLerpFactor = STD20::lerp(FilterBuffers::GetMaxTranLerpFactor(),
                                 tranLerpFactor,
                                 tranLerpProperties.tranLerpToMaxSwitchMult);
  }

  m_filterBuffers.SetTranLerpFactor(tranLerpFactor);
}

} // namespace GOOM::FILTER_FX
