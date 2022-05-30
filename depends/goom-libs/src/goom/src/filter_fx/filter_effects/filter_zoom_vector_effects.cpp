#include "filter_zoom_vector_effects.h"

#include "filter_fx/filter_settings.h"
#include "filter_fx/normalized_coords.h"
#include "utils/name_value_pairs.h"

#include <cstdint>
#include <string>

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

using UTILS::GetPair;
using UTILS::MoveNameValuePairs;
using UTILS::NameValuePairs;
using UTILS::MATH::IGoomRand;

ZoomVectorEffects::ZoomVectorEffects(const uint32_t screenWidth,
                                     const std::string& resourcesDirectory,
                                     const IGoomRand& goomRand,
                                     const NormalizedCoordsConverter& normalizedCoordsConverter,
                                     const GetTheEffectsFunc& getTheExtraEffects) noexcept
  : m_screenWidth{screenWidth},
    m_normalizedCoordsConverter{normalizedCoordsConverter},
    m_theEffects{getTheExtraEffects(resourcesDirectory, goomRand)}
{
}

auto ZoomVectorEffects::GetStandardExtraEffects(const std::string& resourcesDirectory,
                                                const IGoomRand& goomRand) -> TheExtraEffects
{
  return {
      std::make_unique<Hypercos>(goomRand),
      std::make_unique<ImageVelocity>(resourcesDirectory, goomRand),
      std::make_unique<Noise>(goomRand),
      std::make_unique<Planes>(goomRand),
      std::make_unique<Rotation>(goomRand),
      std::make_unique<TanEffect>(goomRand),
  };
}

auto ZoomVectorEffects::SetFilterSettings(const ZoomFilterEffectsSettings& filterEffectsSettings)
    -> void
{
  m_filterEffectsSettings = &filterEffectsSettings;

  m_filterEffectsSettings->speedCoefficientsEffect->SetRandomParams();

  SetRandomHypercosOverlayEffects();
  SetRandomImageVelocityEffects();
  SetRandomNoiseSettings();
  SetRandomPlaneEffects();
  SetRandomRotationSettings();
  SetRandomTanEffects();
}

inline auto ZoomVectorEffects::SetRandomImageVelocityEffects() -> void
{
  if (!m_filterEffectsSettings->imageVelocityEffect)
  {
    return;
  }

  m_theEffects.imageVelocity->SetRandomParams();
}

inline auto ZoomVectorEffects::SetRandomNoiseSettings() -> void
{
  if (!m_filterEffectsSettings->noiseEffect)
  {
    return;
  }

  m_theEffects.noise->SetRandomParams();
}

inline auto ZoomVectorEffects::SetRandomPlaneEffects() -> void
{
  if (!m_filterEffectsSettings->planeEffect)
  {
    return;
  }

  m_theEffects.planes->SetRandomParams(m_filterEffectsSettings->zoomMidpoint, m_screenWidth);
}

inline auto ZoomVectorEffects::SetRandomRotationSettings() -> void
{
  if (!m_filterEffectsSettings->rotationEffect)
  {
    return;
  }

  using Adj = RotationAdjustments::AdjustmentType;

  switch (m_filterEffectsSettings->rotationAdjustments.GetAdjustmentType())
  {
    case Adj::NONE:
      m_theEffects.rotation->SetRandomParams();
      break;
    case Adj::INSTEAD_OF_RANDOM:
      m_theEffects.rotation->ApplyAdjustments(m_filterEffectsSettings->rotationAdjustments);
      break;
    case Adj::AFTER_RANDOM:
      m_theEffects.rotation->SetRandomParams();
      m_theEffects.rotation->ApplyAdjustments(m_filterEffectsSettings->rotationAdjustments);
      break;
    default:
      throw std::logic_error("RotationAdjustments::AdjustmentType not implemented.");
  }
}

inline auto ZoomVectorEffects::SetRandomTanEffects() -> void
{
  if (!m_filterEffectsSettings->tanEffect)
  {
    return;
  }

  m_theEffects.tanEffect->SetRandomParams();
}

auto ZoomVectorEffects::SetRandomHypercosOverlayEffects() -> void
{
  switch (m_filterEffectsSettings->hypercosOverlay)
  {
    case HypercosOverlay::NONE:
      m_theEffects.hypercos->SetDefaultParams();
      break;
    case HypercosOverlay::MODE0:
      m_theEffects.hypercos->SetMode0RandomParams();
      break;
    case HypercosOverlay::MODE1:
      m_theEffects.hypercos->SetMode1RandomParams();
      break;
    case HypercosOverlay::MODE2:
      m_theEffects.hypercos->SetMode2RandomParams();
      break;
    case HypercosOverlay::MODE3:
      m_theEffects.hypercos->SetMode3RandomParams();
      break;
    default:
      break;
  }
}

auto ZoomVectorEffects::GetCleanedVelocity(const NormalizedCoords& velocity) const
    -> NormalizedCoords
{
  return {GetMinVelocityVal(velocity.GetX()), GetMinVelocityVal(velocity.GetY())};
}

inline auto ZoomVectorEffects::GetMinVelocityVal(const float velocityVal) const -> float
{
  if (std::fabs(velocityVal) < m_normalizedCoordsConverter.GetMinNormalizedCoordVal())
  {
    return velocityVal < 0.0F ? -m_normalizedCoordsConverter.GetMinNormalizedCoordVal()
                              : +m_normalizedCoordsConverter.GetMinNormalizedCoordVal();
  }
  return velocityVal;
}

auto ZoomVectorEffects::GetZoomEffectsNameValueParams() const -> NameValuePairs
{
  NameValuePairs nameValuePairs{};

  MoveNameValuePairs(GetHypercosNameValueParams(), nameValuePairs);
  MoveNameValuePairs(GetImageVelocityNameValueParams(), nameValuePairs);
  MoveNameValuePairs(GetNoiseNameValueParams(), nameValuePairs);
  MoveNameValuePairs(GetPlaneNameValueParams(), nameValuePairs);
  MoveNameValuePairs(GetRotationNameValueParams(), nameValuePairs);
  MoveNameValuePairs(GetSpeedCoefficientsNameValueParams(), nameValuePairs);
  MoveNameValuePairs(GetTanEffectNameValueParams(), nameValuePairs);

  return nameValuePairs;
}

inline auto ZoomVectorEffects::GetSpeedCoefficientsNameValueParams() const -> NameValuePairs
{
  return m_filterEffectsSettings->speedCoefficientsEffect
      ->GetSpeedCoefficientsEffectNameValueParams();
}

inline auto ZoomVectorEffects::GetHypercosNameValueParams() const -> NameValuePairs
{
  return m_theEffects.hypercos->GetNameValueParams(PARAM_GROUP);
}

inline auto ZoomVectorEffects::GetImageVelocityNameValueParams() const -> NameValuePairs
{
  NameValuePairs nameValuePairs{
      GetPair(PARAM_GROUP, "imageVelocity", m_filterEffectsSettings->imageVelocityEffect)};
  if (m_filterEffectsSettings->imageVelocityEffect)
  {
    MoveNameValuePairs(m_theEffects.imageVelocity->GetNameValueParams(PARAM_GROUP), nameValuePairs);
  }
  return nameValuePairs;
}

inline auto ZoomVectorEffects::GetNoiseNameValueParams() const -> NameValuePairs
{
  NameValuePairs nameValuePairs{
      GetPair(PARAM_GROUP, "noiseEffect", m_filterEffectsSettings->noiseEffect)};
  if (m_filterEffectsSettings->noiseEffect)
  {
    MoveNameValuePairs(m_theEffects.noise->GetNameValueParams(PARAM_GROUP), nameValuePairs);
  }
  return nameValuePairs;
}

inline auto ZoomVectorEffects::GetPlaneNameValueParams() const -> NameValuePairs
{
  NameValuePairs nameValuePairs{
      GetPair(PARAM_GROUP, "planeEffect", m_filterEffectsSettings->planeEffect)};
  if (m_filterEffectsSettings->planeEffect)
  {
    MoveNameValuePairs(m_theEffects.planes->GetNameValueParams(PARAM_GROUP), nameValuePairs);
  }
  return nameValuePairs;
}

inline auto ZoomVectorEffects::GetRotationNameValueParams() const -> NameValuePairs
{
  NameValuePairs nameValuePairs{
      GetPair(PARAM_GROUP, "rotation", m_filterEffectsSettings->rotationEffect)};
  if (m_filterEffectsSettings->rotationEffect)
  {
    MoveNameValuePairs(m_theEffects.rotation->GetNameValueParams(PARAM_GROUP), nameValuePairs);
  }
  return nameValuePairs;
}

inline auto ZoomVectorEffects::GetTanEffectNameValueParams() const -> NameValuePairs
{
  NameValuePairs nameValuePairs{
      GetPair(PARAM_GROUP, "tanEffect", m_filterEffectsSettings->tanEffect)};
  if (m_filterEffectsSettings->tanEffect)
  {
    MoveNameValuePairs(m_theEffects.tanEffect->GetNameValueParams(PARAM_GROUP), nameValuePairs);
  }
  return nameValuePairs;
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
