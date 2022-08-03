#include "filter_zoom_vector.h"

#include "filter_effects/filter_zoom_vector_effects.h"
#include "normalized_coords.h"
#include "utils/math/misc.h"
#include "utils/name_value_pairs.h"

#include <cstdint>
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
    const ZoomVectorEffects::GetTheEffectsFunc& getTheExtraEffects) noexcept
  : m_zoomVectorEffects{
        screenWidth, resourcesDirectory, goomRand, normalizedCoordsConverter, getTheExtraEffects}
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
  return coords - GetZoomPointVelocity(coords);
}

inline auto FilterZoomVector::GetZoomPointVelocity(const NormalizedCoords& coords) const
    -> NormalizedCoords
{
  const auto sqDistFromZero = SqDistance(coords.GetX(), coords.GetY());

  const auto baseVelocity = m_zoomVectorEffects.GetSpeedCoeffVelocity(sqDistFromZero, coords);

  const auto adjustedVelocity =
      GetZoomEffectsAdjustedVelocity(coords, sqDistFromZero, baseVelocity);

  return m_zoomVectorEffects.GetCleanedVelocity(adjustedVelocity);
}

auto FilterZoomVector::GetZoomEffectsAdjustedVelocity(const NormalizedCoords& coords,
                                                      const float sqDistFromZero,
                                                      const NormalizedCoords& velocity) const
    -> NormalizedCoords
{
  auto newVelocity = velocity;

  if (m_zoomVectorEffects.IsImageVelocityActive())
  {
    newVelocity = m_zoomVectorEffects.GetImageVelocity(coords, newVelocity);
  }

  if (m_zoomVectorEffects.IsRotationActive())
  {
    newVelocity = m_zoomVectorEffects.GetRotatedVelocity(newVelocity);
  }

  if (m_zoomVectorEffects.IsTanEffectActive())
  {
    newVelocity = m_zoomVectorEffects.GetTanEffectVelocity(sqDistFromZero, newVelocity);
  }

  if (m_zoomVectorEffects.IsNoiseActive())
  {
    newVelocity = m_zoomVectorEffects.GetNoiseVelocity(velocity);
  }

  if (m_zoomVectorEffects.IsHypercosOverlayActive())
  {
    newVelocity = m_zoomVectorEffects.GetHypercosVelocity(coords, newVelocity);
  }

  if (m_zoomVectorEffects.IsHorizontalPlaneVelocityActive())
  {
    newVelocity.SetX(m_zoomVectorEffects.GetHorizontalPlaneVelocity(coords, newVelocity));
  }

  if (m_zoomVectorEffects.IsVerticalPlaneVelocityActive())
  {
    newVelocity.SetY(m_zoomVectorEffects.GetVerticalPlaneVelocity(coords, newVelocity));
  }

  /* TODO : Water Mode */
  //    if (data->waveEffect)

  /**
  if (m_filterSettings->mode == ZoomFilterMode::NORMAL_MODE)
    if (ProbabilityOfMInN(1, 2))
      velocity = {-2.0F * xNormalized + velocity.x, -2.0F * yNormalized + velocity.y};
  **/

  return newVelocity;
}

} // namespace GOOM::FILTER_FX
