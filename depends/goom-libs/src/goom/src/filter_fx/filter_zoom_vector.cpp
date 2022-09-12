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

auto FilterZoomVector::GetZoomPoint(const NormalizedCoords& coords) const -> NormalizedCoords
{
  const auto sqDistFromZero = SqDistance(coords.GetX(), coords.GetY());

  const auto zoomCoefficients = m_zoomVectorEffects.GetZoomCoefficients(coords, sqDistFromZero);
  const auto zoomFactor       = 1.0F - zoomCoefficients;
  const auto zoomPoint        = zoomFactor * coords;

  const auto zoomVelocity = coords - zoomPoint;
  const auto afterEffectsVelocity =
      m_zoomVectorEffects.GetAfterEffectsVelocity(coords, sqDistFromZero, zoomVelocity);
  const auto afterEffectsZoomPoint = zoomPoint - afterEffectsVelocity;

  return m_zoomVectorEffects.GetCleanedCoords(afterEffectsZoomPoint);
}

} // namespace GOOM::FILTER_FX
