#include "filter_zoom_vector_effects.h"

#include "filter_data.h"
#include "filter_hypercos.h"
#include "filter_normalized_coords.h"
#include "filter_planes.h"
#include "goomutils/goomrand.h"
#include "goomutils/mathutils.h"
#include "v2d.h"

#include <cmath>
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

using UTILS::GetRandInRange;
using UTILS::m_half_pi;

ZoomVectorEffects::ZoomVectorEffects(const std::string& resourcesDirectory) noexcept
  : m_resourcesDirectory{resourcesDirectory},
    m_hypercos{std::make_unique<Hypercos>()},
    m_planes{std::make_unique<Planes>()}
{
}

ZoomVectorEffects::~ZoomVectorEffects() noexcept = default;

// TODO This may change filter buffers stripe when it shouldn't
void ZoomVectorEffects::SetSpeedCoefficientsEffect(
    const std::shared_ptr<const SpeedCoefficientsEffect> val)
{
  m_speedCoefficientsEffect = val;
}

void ZoomVectorEffects::SetFilterSettings(const ZoomFilterData& filterSettings)
{
  m_filterSettings = &filterSettings;

  SetHypercosOverlaySettings();
}

void ZoomVectorEffects::SetRandomPlaneEffects(const V2dInt& zoomMidPoint,
                                              const uint32_t screenWidth)
{
  m_planes->SetRandomParams(zoomMidPoint, screenWidth);
}

void ZoomVectorEffects::SetHypercosOverlaySettings()
{
  switch (m_filterSettings->hypercosOverlay)
  {
    case HypercosOverlay::NONE:
      m_hypercos->SetDefaultParams();
      break;
    case HypercosOverlay::MODE0:
      m_hypercos->SetMode0RandomParams();
      break;
    case HypercosOverlay::MODE1:
      m_hypercos->SetMode1RandomParams();
      break;
    case HypercosOverlay::MODE2:
      m_hypercos->SetMode2RandomParams();
      break;
    case HypercosOverlay::MODE3:
      m_hypercos->SetMode3RandomParams();
      break;
  }
}

auto ZoomVectorEffects::GetCleanedVelocity(const NormalizedCoords& velocity) -> NormalizedCoords
{
  return {GetMinVelocityVal(velocity.GetX()), GetMinVelocityVal(velocity.GetY())};
}

inline auto ZoomVectorEffects::GetMinVelocityVal(const float velocityVal) -> float
{
  if (std::fabs(velocityVal) < NormalizedCoords::GetMinNormalizedCoordVal())
  {
    return velocityVal < 0.0F ? -NormalizedCoords::GetMinNormalizedCoordVal()
                              : NormalizedCoords::GetMinNormalizedCoordVal();
  }
  return velocityVal;
}

auto ZoomVectorEffects::GetHypercosVelocity(const NormalizedCoords& coords) const
    -> NormalizedCoords
{
  return m_hypercos->GetVelocity(coords);
}

auto ZoomVectorEffects::IsHorizontalPlaneVelocityActive() const -> bool
{
  return m_planes->IsHorizontalPlaneVelocityActive();
}

auto ZoomVectorEffects::GetHorizontalPlaneVelocity(const NormalizedCoords& coords) const -> float
{
  return m_planes->GetHorizontalPlaneVelocity(coords);
}

auto ZoomVectorEffects::IsVerticalPlaneVelocityActive() const -> bool
{
  return m_planes->IsVerticalPlaneVelocityActive();
}

auto ZoomVectorEffects::GetVerticalPlaneVelocity(const NormalizedCoords& coords) const -> float
{
  return m_planes->GetVerticalPlaneVelocity(coords);
}

auto ZoomVectorEffects::GetNoiseVelocity() const -> NormalizedCoords
{
  //    const float xAmp = 1.0/getRandInRange(50.0f, 200.0f);
  //    const float yAmp = 1.0/getRandInRange(50.0f, 200.0f);
  const float amp = (0.5F * m_filterSettings->noiseFactor) /
                    GetRandInRange(ZoomFilterData::NOISE_MIN, ZoomFilterData::NOISE_MAX);
  return {GetRandInRange(-amp, +amp), GetRandInRange(-amp, +amp)};
}

auto ZoomVectorEffects::GetTanEffectVelocity(const float sqDistFromZero,
                                             const NormalizedCoords& velocity) const
    -> NormalizedCoords
{
  const float tanArg =
      stdnew::clamp(std::fmod(sqDistFromZero, m_half_pi), -0.75F * m_half_pi, 0.75F * m_half_pi);
  const float tanSqDist = std::tan(tanArg);
  return {tanSqDist * velocity.GetX(), tanSqDist * velocity.GetY()};
}

auto ZoomVectorEffects::GetRotatedVelocity(const NormalizedCoords& velocity) const
    -> NormalizedCoords
{
  if (m_filterSettings->rotateSpeed < 0.0F)
  {
    return {-m_filterSettings->rotateSpeed * (velocity.GetX() - velocity.GetY()),
            -m_filterSettings->rotateSpeed * (velocity.GetX() + velocity.GetY())};
  }

  return {m_filterSettings->rotateSpeed * (velocity.GetY() + velocity.GetX()),
          m_filterSettings->rotateSpeed * (velocity.GetY() - velocity.GetX())};
}

#if __cplusplus <= 201402L
} // namespace FILTERS
} // namespace GOOM
#else
} // namespace GOOM::FILTERS
#endif
