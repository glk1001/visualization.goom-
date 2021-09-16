#include "filter_zoom_vector_effects.h"

#include "filter_hypercos.h"
#include "filter_normalized_coords.h"
#include "filter_planes.h"
#include "filter_settings.h"
#include "filter_tan_effect.h"
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

ZoomVectorEffects::ZoomVectorEffects(const uint32_t screenWidth,
                                     const std::string& resourcesDirectory) noexcept
  : m_screenWidth{screenWidth},
    m_imageVelocity{resourcesDirectory},
    m_hypercos{std::make_unique<Hypercos>()},
    m_planes{std::make_unique<Planes>()},
    m_tanEffect{std::make_unique<TanEffect>()}
{
}

void ZoomVectorEffects::SetFilterSettings(const ZoomFilterEffectsSettings& filterEffectsSettings)
{
  m_filterEffectsSettings = &filterEffectsSettings;

  m_filterEffectsSettings->speedCoefficientsEffect->SetRandomParams();

  SetRandomImageVelocityEffects();
  SetHypercosOverlaySettings();
  SetRandomPlaneEffects();
  SetRandomTanEffects();
}

inline void ZoomVectorEffects::SetRandomImageVelocityEffects()
{
  if (m_filterEffectsSettings->imageVelocityEffect)
  {
    m_imageVelocity.SetRandomParams();
  }
}

inline void ZoomVectorEffects::SetRandomPlaneEffects()
{
  if (m_filterEffectsSettings->planeEffect)
  {
    m_planes->SetRandomParams(m_filterEffectsSettings->zoomMidPoint, m_screenWidth);
  }
}

inline void ZoomVectorEffects::SetRandomTanEffects()
{
  if (m_filterEffectsSettings->tanEffect)
  {
    m_tanEffect->SetRandomParams();
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

auto ZoomVectorEffects::GetZoomEffectsNameValueParams() const -> NameValuePairs
{
  NameValuePairs nameValuePairs{};

  MoveNameValuePairs(GetRotateNameValueParams(), nameValuePairs);
  MoveNameValuePairs(GetNoiseNameValueParams(), nameValuePairs);
  MoveNameValuePairs(GetTanEffectNameValueParams(), nameValuePairs);
  MoveNameValuePairs(GetImageVelocityNameValueParams(), nameValuePairs);
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

inline auto ZoomVectorEffects::GetHypercosNameValueParams() const -> NameValuePairs
{
  return m_hypercos->GetNameValueParams(PARAM_GROUP);
}

inline auto ZoomVectorEffects::GetPlaneNameValueParams() const -> NameValuePairs
{
  NameValuePairs nameValuePairs{
      GetPair(PARAM_GROUP, "planeEffect", m_filterEffectsSettings->planeEffect)};
  if (m_filterEffectsSettings->planeEffect)
  {
    MoveNameValuePairs(m_planes->GetNameValueParams(PARAM_GROUP), nameValuePairs);
  }
  return nameValuePairs;
}

inline auto ZoomVectorEffects::GetNoiseNameValueParams() const -> NameValuePairs
{
  if (m_filterEffectsSettings->noisify)
  {
    return {GetPair(PARAM_GROUP, "noise", m_filterEffectsSettings->noisify)};
  }
  return {
      GetPair(PARAM_GROUP, "noise", m_filterEffectsSettings->noisify),
      GetPair(PARAM_GROUP, "noiseFactor", m_filterEffectsSettings->noiseFactor),
  };
}

inline auto ZoomVectorEffects::GetTanEffectNameValueParams() const -> NameValuePairs
{
  NameValuePairs nameValuePairs{
      GetPair(PARAM_GROUP, "tanEffect", m_filterEffectsSettings->tanEffect)};
  if (m_filterEffectsSettings->tanEffect)
  {
    MoveNameValuePairs(m_tanEffect->GetNameValueParams(PARAM_GROUP), nameValuePairs);
  }
  return nameValuePairs;
}

inline auto ZoomVectorEffects::GetImageVelocityNameValueParams() const -> NameValuePairs
{
  NameValuePairs nameValuePairs{
      GetPair(PARAM_GROUP, "imageVelocity", m_filterEffectsSettings->imageVelocityEffect)};
  if (m_filterEffectsSettings->imageVelocityEffect)
  {
    MoveNameValuePairs(m_imageVelocity.GetNameValueParams(PARAM_GROUP), nameValuePairs);
  }
  return nameValuePairs;
}

inline auto ZoomVectorEffects::GetRotateNameValueParams() const -> NameValuePairs
{
  return m_filterEffectsSettings->rotation->GetNameValueParams(PARAM_GROUP);
}

#if __cplusplus <= 201402L
} // namespace FILTERS
} // namespace GOOM
#else
} // namespace GOOM::FILTERS
#endif
