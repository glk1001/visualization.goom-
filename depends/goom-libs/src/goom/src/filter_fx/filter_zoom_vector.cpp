module;

//#undef NO_LOGGING

#include <cstdint>
#include <string>

module Goom.FilterFx.FilterZoomVector;

import Goom.FilterFx.FilterEffects.ZoomVectorEffects;
import Goom.FilterFx.FilterSettings;
import Goom.FilterFx.NormalizedCoords;
import Goom.Lib.Point2d;

namespace GOOM::FILTER_FX
{

using FILTER_EFFECTS::ZoomVectorEffects;
using UTILS::NameValuePairs;
using UTILS::MATH::GoomRand;

FilterZoomVector::FilterZoomVector(
    const uint32_t screenWidth,
    const std::string& resourcesDirectory,
    const GoomRand& goomRand,
    const ZoomVectorEffects::GetAfterEffectsFunc& getAfterEffects) noexcept
  : m_zoomVectorEffects{screenWidth, resourcesDirectory, goomRand, getAfterEffects}
{
}

auto FilterZoomVector::SetFilterEffectsSettings(
    const FilterEffectsSettings& filterEffectsSettings) noexcept -> void
{
  m_zoomVectorEffects.SetFilterSettings(filterEffectsSettings);
}

auto FilterZoomVector::GetZoomPoint(const NormalizedCoords& coords) const noexcept
    -> NormalizedCoords
{
  const auto filterEffectsZoomPoint = GetFilterEffectsZoomPoint(coords);
  const auto afterEffectsVelocity   = GetAfterEffectsVelocity(coords, filterEffectsZoomPoint);

  return filterEffectsZoomPoint - afterEffectsVelocity;
}

inline auto FilterZoomVector::GetFilterEffectsZoomPoint(
    const NormalizedCoords& coords) const noexcept -> NormalizedCoords
{
  const auto zoomAdjustment = m_zoomVectorEffects.GetZoomAdjustment(coords);

  const auto& multiplierEffect = m_zoomVectorEffects.GetMultiplierEffect(coords, zoomAdjustment);

  const auto zoomVelocity = NormalizedCoords{-multiplierEffect.x * zoomAdjustment.x,
                                             -multiplierEffect.y * zoomAdjustment.y};

  return coords + zoomVelocity;
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

auto FilterZoomVector::GetNameValueParams() const noexcept -> NameValuePairs
{
  return m_zoomVectorEffects.GetZoomEffectsNameValueParams();
}

auto FilterZoomVector::GetAfterEffectsNameValueParams() const noexcept -> NameValuePairs
{
  return m_zoomVectorEffects.GetAfterEffectsNameValueParams();
}

} // namespace GOOM::FILTER_FX
