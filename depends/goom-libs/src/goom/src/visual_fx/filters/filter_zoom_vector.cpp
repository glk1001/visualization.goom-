#include "filter_zoom_vector.h"

#include "filter_zoom_vector_effects.h"
#include "normalized_coords.h"
#include "utils/mathutils.h"
#include "utils/name_value_pairs.h"

#include <cstdint>
#include <string>

namespace GOOM::VISUAL_FX::FILTERS
{

using UTILS::IGoomRand;
using UTILS::NameValuePairs;
using UTILS::SqDistance;

FilterZoomVector::FilterZoomVector(
    const uint32_t screenWidth,
    const std::string& resourcesDirectory,
    const IGoomRand& goomRand,
    const ZoomVectorEffects::GetTheEffectsFunc& getTheExtraEffects) noexcept
  : m_zoomVectorEffects{screenWidth, resourcesDirectory, goomRand, getTheExtraEffects}
{
}

void FilterZoomVector::SetFilterSettings(const ZoomFilterEffectsSettings& filterEffectsSettings)
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

  const float sqDistFromZero = SqDistance(coords.GetX(), coords.GetY());

  const NormalizedCoords baseVelocity =
      m_zoomVectorEffects.GetSpeedCoeffVelocity(sqDistFromZero, coords);
  const NormalizedCoords adjustedVelocity =
      GetZoomEffectsAdjustedVelocity(sqDistFromZero, coords, baseVelocity);
  const NormalizedCoords cleanedVelocity = ZoomVectorEffects::GetCleanedVelocity(adjustedVelocity);

  return coords - cleanedVelocity;
}

auto FilterZoomVector::GetZoomEffectsAdjustedVelocity(const float sqDistFromZero,
                                                      const NormalizedCoords& coords,
                                                      const NormalizedCoords& velocity) const
    -> NormalizedCoords
{
  NormalizedCoords newVelocity = velocity;

  if (m_zoomVectorEffects.IsImageVelocityActive())
  {
    newVelocity += m_zoomVectorEffects.GetImageVelocity(coords);
  }

  if (m_zoomVectorEffects.IsRotateActive())
  {
    newVelocity = m_zoomVectorEffects.GetRotatedVelocity(newVelocity);
  }

  if (m_zoomVectorEffects.IsTanEffectActive())
  {
    newVelocity = m_zoomVectorEffects.GetTanEffectVelocity(sqDistFromZero, newVelocity);
  }

  if (m_zoomVectorEffects.IsNoiseActive())
  {
    newVelocity += m_zoomVectorEffects.GetNoiseVelocity();
  }

  if (m_zoomVectorEffects.IsHypercosOverlayActive())
  {
    newVelocity += m_zoomVectorEffects.GetHypercosVelocity(coords);
  }

  if (m_zoomVectorEffects.IsHorizontalPlaneVelocityActive())
  {
    newVelocity.SetX(newVelocity.GetX() + m_zoomVectorEffects.GetHorizontalPlaneVelocity(coords));
  }

  if (m_zoomVectorEffects.IsVerticalPlaneVelocityActive())
  {
    newVelocity.SetY(newVelocity.GetY() + m_zoomVectorEffects.GetVerticalPlaneVelocity(coords));
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

} // namespace GOOM::VISUAL_FX::FILTERS
