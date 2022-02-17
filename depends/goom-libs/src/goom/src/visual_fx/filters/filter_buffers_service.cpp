#include "filter_buffers_service.h"

//#undef NO_LOGGING

#include "filter_buffers.h"
#include "filter_settings.h"
#include "goom/logging.h"
#include "goom_plugin_info.h"
#include "normalized_coords.h"
#include "speed_coefficients_effect.h"
#include "utils/math/mathutils.h"
#include "utils/name_value_pairs.h"
#include "utils/parallel_utils.h"
#include "zoom_vector.h"

#undef NDEBUG
#include <cassert>
#include <cstdint>

namespace GOOM::VISUAL_FX::FILTERS
{

using FILTERS::IZoomVector;
using FILTERS::NormalizedCoords;
using FILTERS::ZoomFilterBuffers;
using UTILS::Logging;
using UTILS::NameValuePairs;
using UTILS::Parallel;
using UTILS::MATH::FloatsEqual;

FilterBuffersService::FilterBuffersService(
    Parallel& parallel,
    const PluginInfo& goomInfo,
    const NormalizedCoordsConverter& normalizedCoordsConverter,
    std::unique_ptr<IZoomVector> zoomVector) noexcept
  : m_zoomVector{std::move(zoomVector)},
    m_filterBuffers{parallel, goomInfo, normalizedCoordsConverter,
                    [this](const NormalizedCoords& normalizedCoords)
                    { return m_zoomVector->GetZoomPoint(normalizedCoords); }}
{
}

void FilterBuffersService::SetFilterBufferSettings(
    const ZoomFilterBufferSettings& filterBufferSettings)
{
  UpdateTranLerpFactor(filterBufferSettings.tranLerpIncrement,
                       filterBufferSettings.tranLerpToMaxSwitchMult);
}

void FilterBuffersService::SetFilterEffectsSettings(
    const ZoomFilterEffectsSettings& filterEffectsSettings)
{
  m_nextFilterEffectsSettings = filterEffectsSettings;
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

  m_filterBuffers.SetBuffMidpoint(m_currentFilterEffectsSettings.zoomMidpoint);
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
    tranLerpFactor = std::clamp(tranLerpFactor + tranLerpIncrement, 0,
                                ZoomFilterBuffers::GetMaxTranLerpFactor());
  }

  if (!FloatsEqual(tranLerpToMaxSwitchMult, 1.0F))
  {
    tranLerpFactor = static_cast<int32_t>(
        STD20::lerp(static_cast<float>(ZoomFilterBuffers::GetMaxTranLerpFactor()),
                    static_cast<float>(tranLerpFactor), tranLerpToMaxSwitchMult));
  }

  m_filterBuffers.SetTranLerpFactor(tranLerpFactor);
}

} // namespace GOOM::VISUAL_FX::FILTERS
