#include "zoom_vector_effects.h"

#include "filter_fx/after_effects/zoom_vector_after_effects.h"
#include "filter_fx/filter_settings.h"
#include "filter_fx/normalized_coords.h"
#include "utils/name_value_pairs.h"

#include <cstdint>
#include <string>

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

using AFTER_EFFECTS::AfterEffects;
using GOOM::UTILS::NameValuePairs;
using GOOM::UTILS::MATH::IGoomRand;

ZoomVectorEffects::ZoomVectorEffects(const uint32_t screenWidth,
                                     const std::string& resourcesDirectory,
                                     const IGoomRand& goomRand,
                                     const NormalizedCoordsConverter& normalizedCoordsConverter,
                                     const GetAfterEffectsFunc& getAfterEffects) noexcept
  : m_normalizedCoordsConverter{normalizedCoordsConverter},
    m_zoomVectorAfterEffects{screenWidth, getAfterEffects(goomRand, resourcesDirectory)}
{
}

auto ZoomVectorEffects::GetStandardAfterEffects(const IGoomRand& goomRand,
                                                const std::string& resourcesDirectory)
    -> AfterEffects
{
  return AFTER_EFFECTS::GetStandardAfterEffects(goomRand, resourcesDirectory);
}

auto ZoomVectorEffects::SetFilterSettings(const ZoomFilterEffectsSettings& filterEffectsSettings)
    -> void
{
  m_filterEffectsSettings = &filterEffectsSettings;
  m_filterEffectsSettings->speedCoefficientsEffect->SetRandomParams();

  m_zoomVectorAfterEffects.SetAfterEffectsSettings(m_filterEffectsSettings->afterEffectsSettings,
                                                   m_filterEffectsSettings->zoomMidpoint);
}

auto ZoomVectorEffects::GetCleanedCoords(const NormalizedCoords& coords) const -> NormalizedCoords
{
  return {GetMinCoordVal(coords.GetX(), m_normalizedCoordsConverter.GetXMinNormalizedCoordVal()),
          GetMinCoordVal(coords.GetY(), m_normalizedCoordsConverter.GetYMinNormalizedCoordVal())};
}

inline auto ZoomVectorEffects::GetMinCoordVal(const float coordVal,
                                              const float minNormalizedCoordVal) -> float
{
  if (std::fabs(coordVal) < minNormalizedCoordVal)
  {
    return coordVal < 0.0F ? -minNormalizedCoordVal : +minNormalizedCoordVal;
  }
  return coordVal;
}

auto ZoomVectorEffects::GetZoomCoefficientsNameValueParams() const -> NameValuePairs
{
  return m_filterEffectsSettings->speedCoefficientsEffect
      ->GetSpeedCoefficientsEffectNameValueParams();
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
