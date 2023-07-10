//#undef NO_LOGGING

#include "filter_zoom_vector.h"

#include "filter_effects/zoom_vector_effects.h"
#include "goom_config.h"
#include "goom_logger.h"
#include "normalized_coords.h"
#include "utils/math/misc.h"

#include <string>

namespace GOOM::FILTER_FX
{

using FILTER_EFFECTS::ZoomVectorEffects;
using UTILS::NameValuePairs;
using UTILS::MATH::IGoomRand;
using UTILS::MATH::SqDistance;

FilterZoomVector::FilterZoomVector(
    const uint32_t screenWidth,
    const std::string& resourcesDirectory,
    const IGoomRand& goomRand,
    const NormalizedCoordsConverter& normalizedCoordsConverter,
    const ZoomVectorEffects::GetAfterEffectsFunc& getAfterEffects) noexcept
  : m_zoomVectorEffects{
        screenWidth, resourcesDirectory, goomRand, normalizedCoordsConverter, getAfterEffects}
{
}

auto FilterZoomVector::SetFilterEffectsSettings(
    const FilterEffectsSettings& filterEffectsSettings) noexcept -> void
{
  m_zoomVectorEffects.SetFilterSettings(filterEffectsSettings);
}

auto FilterZoomVector::GetNameValueParams(
    [[maybe_unused]] const std::string& paramGroup) const noexcept -> NameValuePairs
{
  return m_zoomVectorEffects.GetZoomEffectsNameValueParams();
}

auto FilterZoomVector::GetZoomInPoint(const NormalizedCoords& coords,
                                      const NormalizedCoords& filterViewportCoords) const noexcept
    -> NormalizedCoords
{
  const auto filterEffectsZoomInPoint = GetFilterEffectsZoomInPoint(coords, filterViewportCoords);
  const auto afterEffectsVelocity     = GetAfterEffectsVelocity(coords, filterEffectsZoomInPoint);

  const auto finalZoomInPoint =
      filterEffectsZoomInPoint -
      (m_zoomVectorEffects.GetAfterEffectsVelocityContribution() * afterEffectsVelocity);

  return m_zoomVectorEffects.GetCleanedCoords(finalZoomInPoint);
}

inline auto FilterZoomVector::GetFilterEffectsZoomInPoint(
    // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
    const NormalizedCoords& coords,
    const NormalizedCoords& filterViewportCoords) const noexcept -> NormalizedCoords
{
  const auto absSqFilterViewportCoords =
      SqDistance(filterViewportCoords.GetX(), filterViewportCoords.GetY());
  const auto zoomInCoeffs =
      m_zoomVectorEffects.GetZoomInCoefficients(filterViewportCoords, absSqFilterViewportCoords);
  const auto zoomInFactor = 1.0F - zoomInCoeffs;

  return zoomInFactor * coords;
}

inline auto FilterZoomVector::GetAfterEffectsVelocity(
    // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
    const NormalizedCoords& coords,
    const NormalizedCoords& filterEffectsZoomInPoint) const noexcept -> NormalizedCoords
{
  const auto zoomInVelocity = coords - filterEffectsZoomInPoint;
  const auto absSqCoords    = SqDistance(coords.GetX(), coords.GetY());
  return m_zoomVectorEffects.GetAfterEffectsVelocity(coords, absSqCoords, zoomInVelocity);
}

} // namespace GOOM::FILTER_FX
