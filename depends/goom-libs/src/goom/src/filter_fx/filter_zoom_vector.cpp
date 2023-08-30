//#undef NO_LOGGING

#include "filter_zoom_vector.h"

#include "filter_effects/zoom_vector_effects.h"
#include "goom_config.h"
#include "goom_logger.h"
#include "normalized_coords.h"

#include <string>

namespace GOOM::FILTER_FX
{

using FILTER_EFFECTS::ZoomVectorEffects;
using UTILS::NameValuePairs;
using UTILS::MATH::IGoomRand;

FilterZoomVector::FilterZoomVector(
    const uint32_t screenWidth,
    const std::string& resourcesDirectory,
    const IGoomRand& goomRand,
    const ZoomVectorEffects::GetAfterEffectsFunc& getAfterEffects) noexcept
  : m_zoomVectorEffects{screenWidth, resourcesDirectory, goomRand, getAfterEffects}
{
}

auto FilterZoomVector::SetFilterEffectsSettings(
    const FilterEffectsSettings& filterEffectsSettings) noexcept -> void
{
  m_zoomVectorEffects.SetFilterSettings(filterEffectsSettings);
}

auto FilterZoomVector::GetZoomPoint(const NormalizedCoords& coords,
                                    const NormalizedCoords& filterViewportCoords) const noexcept
    -> NormalizedCoords
{
  const auto filterEffectsZoomPoint = GetFilterEffectsZoomPoint(coords, filterViewportCoords);
  const auto afterEffectsVelocity   = GetAfterEffectsVelocity(coords, filterEffectsZoomPoint);

  return filterEffectsZoomPoint - afterEffectsVelocity;
}

inline auto FilterZoomVector::GetFilterEffectsZoomPoint(
    // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
    const NormalizedCoords& coords,
    const NormalizedCoords& filterViewportCoords) const noexcept -> NormalizedCoords
{
  const auto zoomAdjustment = m_zoomVectorEffects.GetZoomAdjustment(
      filterViewportCoords, SqDistanceFromZero(filterViewportCoords));

  const auto zoomFactor = 1.0F - zoomAdjustment;

  return zoomFactor * coords;
}

inline auto FilterZoomVector::GetAfterEffectsVelocity(
    // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
    const NormalizedCoords& coords,
    const NormalizedCoords& zoomPoint) const noexcept -> NormalizedCoords
{
  const auto zoomVelocity       = coords - zoomPoint;
  const auto sqDistanceFromZero = SqDistanceFromZero(coords);

  return m_zoomVectorEffects.GetAfterEffectsVelocityMultiplier() *
         m_zoomVectorEffects.GetAfterEffectsVelocity(coords, sqDistanceFromZero, zoomVelocity);
}

auto FilterZoomVector::GetNameValueParams(
    [[maybe_unused]] const std::string& paramGroup) const noexcept -> NameValuePairs
{
  return m_zoomVectorEffects.GetZoomEffectsNameValueParams();
}

} // namespace GOOM::FILTER_FX
