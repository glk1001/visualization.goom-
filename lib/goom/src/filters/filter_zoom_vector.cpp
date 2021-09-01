#include "filter_zoom_vector.h"

#include "filter_data.h"
#include "filter_normalized_coords.h"
#include "filter_zoom_vector_effects.h"
#include "goomutils/mathutils.h"
#include "v2d.h"

#include <cstdint>
#include <memory>
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

using UTILS::SqDistance;

FilterZoomVector::FilterZoomVector(const std::string& resourcesDirectory) noexcept
  : m_zoomVectorEffects{resourcesDirectory}
{
}

void FilterZoomVector::SetFilterSettings(const ZoomFilterData& filterSettings)
{
  m_zoomVectorEffects.SetFilterSettings(filterSettings);
}

void FilterZoomVector::SetRandomPlaneEffects(const V2dInt& zoomMidPoint, const uint32_t screenWidth)
{
  m_zoomVectorEffects.SetRandomPlaneEffects(zoomMidPoint, screenWidth);
}

void FilterZoomVector::SetSpeedCoefficientsEffect(
    const std::shared_ptr<const SpeedCoefficientsEffect> val)
{
  m_zoomVectorEffects.SetSpeedCoefficientsEffect(val);
}

void FilterZoomVector::SetMaxSpeedCoeff(const float val)
{
  m_zoomVectorEffects.SetMaxSpeedCoeff(val);
}

auto FilterZoomVector::GetZoomPoint(const NormalizedCoords& coords) const -> NormalizedCoords
{
  const float sqDistFromZero = SqDistance(coords.GetX(), coords.GetY());

  NormalizedCoords velocity = m_zoomVectorEffects.GetSpeedCoeffVelocity(sqDistFromZero, coords);

  GetZoomEffectsAdjustedVelocity(sqDistFromZero, coords, velocity);

  return coords - ZoomVectorEffects::GetCleanedVelocity(velocity);
}

void FilterZoomVector::GetZoomEffectsAdjustedVelocity(const float sqDistFromZero,
                                                      const NormalizedCoords& coords,
                                                      NormalizedCoords& velocity) const
{
  if (m_zoomVectorEffects.IsRotateActive())
  {
    velocity = m_zoomVectorEffects.GetRotatedVelocity(velocity);
  }

  if (m_zoomVectorEffects.IsTanEffectActive())
  {
    velocity = m_zoomVectorEffects.GetTanEffectVelocity(sqDistFromZero, velocity);
  }

  if (m_zoomVectorEffects.IsNoiseActive())
  {
    velocity += m_zoomVectorEffects.GetNoiseVelocity();
  }

  if (m_zoomVectorEffects.IsHypercosOverlayActive())
  {
    velocity += m_zoomVectorEffects.GetHypercosVelocity(coords);
  }

  if (m_zoomVectorEffects.IsHorizontalPlaneVelocityActive())
  {
    velocity.SetX(velocity.GetX() + m_zoomVectorEffects.GetHorizontalPlaneVelocity(coords));
  }

  if (m_zoomVectorEffects.IsVerticalPlaneVelocityActive())
  {
    velocity.SetY(velocity.GetY() + m_zoomVectorEffects.GetVerticalPlaneVelocity(coords));
  }

  /* TODO : Water Mode */
  //    if (data->waveEffect)

  /**
  if (m_filterSettings->mode == ZoomFilterMode::NORMAL_MODE)
    if (ProbabilityOfMInN(1, 2))
      velocity = {-2.0F * xNormalized + velocity.x, -2.0F * yNormalized + velocity.y};
  **/
}

#if __cplusplus <= 201402L
} // namespace FILTERS
} // namespace GOOM
#else
} // namespace GOOM::FILTERS
#endif
