#include "filter_buffers_service.h"

#include "filter_buffers.h"
#include "filter_normalized_coords.h"
#include "filter_settings.h"
#include "filter_speed_coefficients_effect.h"
#include "goom_plugin_info.h"
#include "goom_zoom_vector.h"
#include "goomutils/goomrand.h"
#include "goomutils/logging_control.h"
//#undef NO_LOGGING
#include "goomutils/logging.h"
#include "goomutils/mathutils.h"
#include "goomutils/parallel_utils.h"
#undef NDEBUG
#include <cassert>
#include <cstdint>
#include <string>

#if __cplusplus <= 201402L
namespace GOOM
{
namespace FILTERS
{
#else
namespace GOOM::FILTERS
{
#endif

using FILTERS::IZoomVector;
using FILTERS::NormalizedCoords;
using FILTERS::ZoomFilterBuffers;
using UTILS::floats_equal;
using UTILS::GetRandInRange;
using UTILS::Logging;
using UTILS::Parallel;

constexpr float MAX_MAX_SPEED_COEFF = +4.01F;

FilterBuffersService::FilterBuffersService(
    Parallel& parallel,
    const std::shared_ptr<const PluginInfo>& goomInfo,
    std::unique_ptr<IZoomVector> zoomVector) noexcept
  : m_screenWidth{goomInfo->GetScreenInfo().width},
    m_zoomVector{std::move(zoomVector)},
    m_filterBuffers{parallel, goomInfo, [this](const NormalizedCoords& normalizedCoords) {
                      return m_zoomVector->GetZoomPoint(normalizedCoords);
                    }}
{
}

void FilterBuffersService::SetFilterBufferSettings(
    const ZoomFilterBufferSettings& filterBufferSettings)
{
  UpdateTranLerpFactor(filterBufferSettings.tranLerpIncrement,
                       filterBufferSettings.tranLerpToMaxSwitchMult);
}

void FilterBuffersService::SetFilterEffectsSettings(const ZoomFilterEffectsSettings& filterSettings)
{
  m_nextFilterEffectsSettings = filterSettings;
  m_pendingFilterEffectsSettings = true;
}

auto FilterBuffersService::GetNameValueParams(const std::string& paramGroup) const
    -> std::vector<std::pair<std::string, std::string>>
{
  return m_zoomVector->GetNameValueParams(paramGroup);
}

void FilterBuffersService::Start()
{
  m_currentFilterEffectsSettings = m_nextFilterEffectsSettings;
  assert(m_currentFilterEffectsSettings.speedCoefficientsEffect != nullptr);

  UpdateFilterEffectsSettings();

  m_filterBuffers.Start();
}

inline void FilterBuffersService::UpdateFilterEffectsSettings()
{
  m_zoomVector->SetFilterSettings(m_currentFilterEffectsSettings);
  // TODO Random calc should not be here. Move to vector effects
  m_zoomVector->SetMaxSpeedCoeff(GetRandInRange(0.5F, 1.0F) * MAX_MAX_SPEED_COEFF);

  m_filterBuffers.SetBuffMidPoint(m_currentFilterEffectsSettings.zoomMidPoint);
  m_filterBuffers.NotifyFilterSettingsHaveChanged();
}

void FilterBuffersService::UpdateTranBuffers()
{
  m_filterBuffers.UpdateTranBuffers();

  if (AreStartingFreshTranBuffers())
  {
    StartFreshTranBuffers();
  }
}

inline auto FilterBuffersService::AreStartingFreshTranBuffers() const -> bool
{
  return m_filterBuffers.GetTranBuffersState() ==
         ZoomFilterBuffers::TranBuffersState::START_FRESH_TRAN_BUFFERS;
}

void FilterBuffersService::StartFreshTranBuffers()
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

inline void FilterBuffersService::UpdateTranLerpFactor(const int32_t tranLerpIncrement,
                                                       const float tranLerpToMaxSwitchMult)
{
  int32_t tranLerpFactor = m_filterBuffers.GetTranLerpFactor();

  if (tranLerpIncrement != 0)
  {
    tranLerpFactor = stdnew::clamp(tranLerpFactor + tranLerpIncrement, 0,
                                   ZoomFilterBuffers::GetMaxTranLerpFactor());
  }

  if (!floats_equal(tranLerpToMaxSwitchMult, 1.0F))
  {
    tranLerpFactor = static_cast<int32_t>(
        stdnew::lerp(static_cast<float>(ZoomFilterBuffers::GetMaxTranLerpFactor()),
                     static_cast<float>(tranLerpFactor), tranLerpToMaxSwitchMult));
  }

  m_filterBuffers.SetTranLerpFactor(tranLerpFactor);
}

#if __cplusplus <= 201402L
} // namespace FILTERS
} // namespace GOOM
#else
} // namespace GOOM::FILTERS
#endif
