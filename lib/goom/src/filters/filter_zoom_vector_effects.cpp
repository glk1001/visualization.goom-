#include "filter_zoom_vector_effects.h"

#include "filter_hypercos.h"
#include "filter_normalized_coords.h"
#include "filter_planes.h"
#include "filter_settings.h"
#include "goomutils/goomrand.h"
#include "goomutils/mathutils.h"
#include "goomutils/name_value_pairs.h"

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

using UTILS::GetFullParamGroup;
using UTILS::GetPair;
using UTILS::GetRandInRange;
using UTILS::m_half_pi;
using UTILS::MoveNameValuePairs;
using UTILS::NameValuePairs;

ZoomVectorEffects::ZoomVectorEffects(const uint32_t screenWidth) noexcept
  : m_screenWidth{screenWidth},
    m_hypercos{std::make_unique<Hypercos>()},
    m_planes{std::make_unique<Planes>()}
{
}

auto ZoomVectorEffects::GetZoomEffectsNameValueParams() const -> NameValuePairs
{
  NameValuePairs nameValuePairs{};

  MoveNameValuePairs(GetRotateNameValueParams(), nameValuePairs);
  MoveNameValuePairs(GetNoiseNameValueParams(), nameValuePairs);
  MoveNameValuePairs(GetTanEffectNameValueParams(), nameValuePairs);
  MoveNameValuePairs(GetPlaneNameValueParams(), nameValuePairs);
  MoveNameValuePairs(GetHypercosNameValueParams(), nameValuePairs);
  MoveNameValuePairs(GetSpeedCoefficientsEffectNameValueParams(), nameValuePairs);

  return nameValuePairs;
}

inline auto ZoomVectorEffects::GetSpeedCoefficientsEffectNameValueParams() const -> NameValuePairs
{
  return m_filterEffectsSettings->speedCoefficientsEffect
      ->GetSpeedCoefficientsEffectNameValueParams();
}

void ZoomVectorEffects::SetFilterSettings(const ZoomFilterEffectsSettings& filterEffectsSettings)
{
  m_filterEffectsSettings = &filterEffectsSettings;

  m_filterEffectsSettings->speedCoefficientsEffect->SetRandomParams();

  SetHypercosOverlaySettings();
  SetRandomPlaneEffects();
}

inline void ZoomVectorEffects::SetRandomPlaneEffects()
{
  if (m_filterEffectsSettings->planeEffect)
  {
    m_planes->SetRandomParams(m_filterEffectsSettings->zoomMidPoint, m_screenWidth);
  }
}

void ZoomVectorEffects::SetHypercosOverlaySettings()
{
  switch (m_filterEffectsSettings->hypercosOverlay)
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

inline auto ZoomVectorEffects::GetHypercosNameValueParams() const -> NameValuePairs
{
  return m_hypercos->GetNameValueParams(PARAM_GROUP);
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

inline auto ZoomVectorEffects::GetPlaneNameValueParams() const -> NameValuePairs
{
  return m_planes->GetNameValueParams(PARAM_GROUP);
}

auto ZoomVectorEffects::GetNoiseVelocity() const -> NormalizedCoords
{
  //    const float xAmp = 1.0/getRandInRange(50.0f, 200.0f);
  //    const float yAmp = 1.0/getRandInRange(50.0f, 200.0f);
  const float amp =
      (0.5F * m_filterEffectsSettings->noiseFactor) / GetRandInRange(NOISE_MIN, NOISE_MAX);
  return {GetRandInRange(-amp, +amp), GetRandInRange(-amp, +amp)};
}

inline auto ZoomVectorEffects::GetNoiseNameValueParams() const -> NameValuePairs
{
  return {
      GetPair(PARAM_GROUP, "noise", m_filterEffectsSettings->noisify),
      GetPair(PARAM_GROUP, "noiseFactor", m_filterEffectsSettings->noiseFactor),
  };
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

inline auto ZoomVectorEffects::GetTanEffectNameValueParams() const -> NameValuePairs
{
  return {
      GetPair(PARAM_GROUP, "tanEffect", m_filterEffectsSettings->tanEffect),
  };
}

auto ZoomVectorEffects::GetRotatedVelocity(const NormalizedCoords& velocity) const
    -> NormalizedCoords
{
  if (m_filterEffectsSettings->xRotateSpeed < 0.0F)
  {
    return {-m_filterEffectsSettings->xRotateSpeed * (velocity.GetX() - velocity.GetY()),
            -m_filterEffectsSettings->yRotateSpeed * (velocity.GetX() + velocity.GetY())};
  }

  return {m_filterEffectsSettings->xRotateSpeed * (velocity.GetY() + velocity.GetX()),
          m_filterEffectsSettings->yRotateSpeed * (velocity.GetY() - velocity.GetX())};
}

inline auto ZoomVectorEffects::GetRotateNameValueParams() const -> NameValuePairs
{
  return {
      GetPair(PARAM_GROUP, "x rotate speed", m_filterEffectsSettings->xRotateSpeed),
      GetPair(PARAM_GROUP, "y rotate speed", m_filterEffectsSettings->yRotateSpeed),
  };
}

#if __cplusplus <= 201402L
} // namespace FILTERS
} // namespace GOOM
#else
} // namespace GOOM::FILTERS
#endif
