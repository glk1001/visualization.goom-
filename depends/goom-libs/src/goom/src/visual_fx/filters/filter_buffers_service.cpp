#include "filter_buffers_service.h"

#include "filter_buffers.h"
#include "filter_settings.h"
#include "goom/logging_control.h"
#include "goom_plugin_info.h"
#include "normalized_coords.h"
#include "speed_coefficients_effect.h"
#include "utils/randutils.h"
#include "zoom_vector.h"
//#undef NO_LOGGING
#include "goom/logging.h"
#include "utils/mathutils.h"
#include "utils/name_value_pairs.h"
#include "utils/parallel_utils.h"
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
using UTILS::NameValuePairs;
using UTILS::Parallel;

FilterBuffersService::FilterBuffersService(Parallel& parallel,
                                           const PluginInfo& goomInfo,
                                           std::unique_ptr<IZoomVector> zoomVector) noexcept
  : m_zoomVector{std::move(zoomVector)},
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

auto FilterBuffersService::GetNameValueParams(const std::string& paramGroup) const -> NameValuePairs
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
