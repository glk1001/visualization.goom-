#include "filter_zoom_vector.h"

//#undef NO_LOGGING

#include "filter_effects/zoom_vector_effects.h"
#include "logging.h"
#include "normalized_coords.h"
#include "utils/math/misc.h"
#include "utils/name_value_pairs.h"

#include <cstdint>
#include <string>

namespace GOOM::FILTER_FX
{

using FILTER_EFFECTS::ZoomVectorEffects;
using GOOM::UTILS::Logging; // NOLINT(misc-unused-using-decls)
using GOOM::UTILS::NameValuePairs;
using GOOM::UTILS::MATH::IGoomRand;
using GOOM::UTILS::MATH::SqDistance;

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

auto FilterZoomVector::SetFilterSettings(const ZoomFilterEffectsSettings& filterEffectsSettings)
    -> void
{
  m_zoomVectorEffects.SetFilterSettings(filterEffectsSettings);
}

auto FilterZoomVector::GetNameValueParams([[maybe_unused]] const std::string& paramGroup) const
    -> NameValuePairs
{
  return m_zoomVectorEffects.GetZoomEffectsNameValueParams();
}

auto FilterZoomVector::GetZoomInPoint(const NormalizedCoords& coords) const -> NormalizedCoords
{
  const auto sqDistFromZero = SqDistance(coords.GetX(), coords.GetY());

  const auto zoomInCoeffs = m_zoomVectorEffects.GetZoomInCoefficients(coords, sqDistFromZero);
  const auto zoomInFactor = 1.0F - zoomInCoeffs;
  const auto zoomInPoint  = zoomInFactor * coords;

  const auto zoomInVelocity = coords - zoomInPoint;
  const auto afterEffectsVelocity =
      m_zoomVectorEffects.GetAfterEffectsVelocity(coords, sqDistFromZero, zoomInVelocity);
  const auto afterEffectsZoomInPoint = zoomInPoint - afterEffectsVelocity;

  return m_zoomVectorEffects.GetCleanedCoords(afterEffectsZoomInPoint);
}

} // namespace GOOM::FILTER_FX
