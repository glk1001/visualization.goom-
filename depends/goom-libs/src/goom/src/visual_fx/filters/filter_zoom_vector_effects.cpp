#include "filter_zoom_vector_effects.h"

#include "filter_settings.h"
#include "normalized_coords.h"
#include "utils/name_value_pairs.h"

#include <cstdint>
#include <string>

namespace GOOM::VISUAL_FX::FILTERS
{

using UTILS::GetPair;
using UTILS::MoveNameValuePairs;
using UTILS::NameValuePairs;

ZoomVectorEffects::ZoomVectorEffects(const uint32_t screenWidth,
                                     const std::string& resourcesDirectory,
                                     const UTILS::IGoomRand& goomRand) noexcept
  : m_screenWidth{screenWidth},
    m_imageVelocity{resourcesDirectory, goomRand},
    m_noise{goomRand},
    m_hypercos{goomRand},
    m_planes{goomRand},
    m_tanEffect{goomRand}
{
}

void ZoomVectorEffects::SetFilterSettings(const ZoomFilterEffectsSettings& filterEffectsSettings)
{
  m_filterEffectsSettings = &filterEffectsSettings;

  m_filterEffectsSettings->speedCoefficientsEffect->SetRandomParams();

  SetNoiseSettings();
  SetRandomImageVelocityEffects();
  SetRandomHypercosOverlayEffects();
  SetRandomPlaneEffects();
  SetRandomTanEffects();
}

inline void ZoomVectorEffects::SetNoiseSettings()
{
  if (m_filterEffectsSettings->noiseEffect)
  {
    m_noise.SetRandomParams();
  }
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
    m_planes.SetRandomParams(m_filterEffectsSettings->zoomMidPoint, m_screenWidth);
  }
}

inline void ZoomVectorEffects::SetRandomTanEffects()
{
  if (m_filterEffectsSettings->tanEffect)
  {
    m_tanEffect.SetRandomParams();
  }
}

void ZoomVectorEffects::SetRandomHypercosOverlayEffects()
{
  switch (m_filterEffectsSettings->hypercosOverlay)
  {
    case HypercosOverlay::NONE:
      m_hypercos.SetDefaultParams();
      break;
    case HypercosOverlay::MODE0:
      m_hypercos.SetMode0RandomParams();
      break;
    case HypercosOverlay::MODE1:
      m_hypercos.SetMode1RandomParams();
      break;
    case HypercosOverlay::MODE2:
      m_hypercos.SetMode2RandomParams();
      break;
    case HypercosOverlay::MODE3:
      m_hypercos.SetMode3RandomParams();
      break;
    default:
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
  MoveNameValuePairs(GetSpeedCoefficientsNameValueParams(), nameValuePairs);

  return nameValuePairs;
}

inline auto ZoomVectorEffects::GetSpeedCoefficientsNameValueParams() const -> NameValuePairs
{
  return m_filterEffectsSettings->speedCoefficientsEffect
      ->GetSpeedCoefficientsEffectNameValueParams();
}

inline auto ZoomVectorEffects::GetHypercosNameValueParams() const -> NameValuePairs
{
  return m_hypercos.GetNameValueParams(PARAM_GROUP);
}

inline auto ZoomVectorEffects::GetPlaneNameValueParams() const -> NameValuePairs
{
  NameValuePairs nameValuePairs{
      GetPair(PARAM_GROUP, "planeEffect", m_filterEffectsSettings->planeEffect)};
  if (m_filterEffectsSettings->planeEffect)
  {
    MoveNameValuePairs(m_planes.GetNameValueParams(PARAM_GROUP), nameValuePairs);
  }
  return nameValuePairs;
}

inline auto ZoomVectorEffects::GetNoiseNameValueParams() const -> NameValuePairs
{
  NameValuePairs nameValuePairs{
      GetPair(PARAM_GROUP, "noise", m_filterEffectsSettings->noiseEffect)};
  if (m_filterEffectsSettings->noiseEffect)
  {
    MoveNameValuePairs(m_noise.GetNameValueParams(PARAM_GROUP), nameValuePairs);
  }
  return nameValuePairs;
}

inline auto ZoomVectorEffects::GetTanEffectNameValueParams() const -> NameValuePairs
{
  NameValuePairs nameValuePairs{
      GetPair(PARAM_GROUP, "tanEffect", m_filterEffectsSettings->tanEffect)};
  if (m_filterEffectsSettings->tanEffect)
  {
    MoveNameValuePairs(m_tanEffect.GetNameValueParams(PARAM_GROUP), nameValuePairs);
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

} // namespace GOOM::VISUAL_FX::FILTERS
