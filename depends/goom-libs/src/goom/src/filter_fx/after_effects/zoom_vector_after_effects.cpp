#include "zoom_vector_after_effects.h"

#include "filter_fx/filter_settings.h"
#include "filter_fx/normalized_coords.h"
#include "goom_config.h"
#include "utils/name_value_pairs.h"

#include <cstdint>

namespace GOOM::FILTER_FX::AFTER_EFFECTS
{

using GOOM::UTILS::GetPair;
using GOOM::UTILS::MoveNameValuePairs;
using GOOM::UTILS::NameValuePairs;

ZoomVectorAfterEffects::ZoomVectorAfterEffects(const uint32_t screenWidth,
                                               AfterEffects&& afterEffects) noexcept
  : m_screenWidth{screenWidth}, m_afterEffects{std::move(afterEffects)}
{
}

auto ZoomVectorAfterEffects::SetAfterEffectsSettings(
    const AfterEffectsSettings& afterEffectsSettings, const Point2dInt& zoomMidpoint) noexcept
    -> void
{
  m_afterEffectsSettings = afterEffectsSettings;
  m_rotationAdjustments  = afterEffectsSettings.rotationAdjustments; // TODO Fix this
  m_zoomMidpoint         = zoomMidpoint;

  SetRandomHypercosOverlayEffects();
  SetRandomImageVelocityEffects();
  SetRandomNoiseSettings();
  SetRandomPlaneEffects();
  SetRandomRotationSettings();
  SetRandomTanEffects();
}

auto ZoomVectorAfterEffects::GetAfterEffectsVelocity(
    const NormalizedCoords& coords,
    const float sqDistFromZero,
    const NormalizedCoords& velocity) const noexcept -> NormalizedCoords
{
  auto newVelocity = velocity;

  if (m_afterEffectsSettings.imageVelocityEffect)
  {
    newVelocity = m_afterEffects.GetImageVelocity().GetVelocity(coords, newVelocity);
  }

  if (m_afterEffectsSettings.rotationEffect)
  {
    newVelocity = m_afterEffects.GetRotation().GetVelocity(newVelocity);
  }

  if (m_afterEffectsSettings.tanEffect)
  {
    newVelocity = m_afterEffects.GetTanEffect().GetVelocity(sqDistFromZero, newVelocity);
  }

  if (m_afterEffectsSettings.noiseEffect)
  {
    newVelocity = m_afterEffects.GetNoise().GetVelocity(velocity);
  }

  if (m_afterEffectsSettings.hypercosOverlay != HypercosOverlay::NONE)
  {
    newVelocity = m_afterEffects.GetHypercos().GetVelocity(coords, newVelocity);
  }

  if (m_afterEffects.GetPlanes().IsHorizontalPlaneVelocityActive())
  {
    newVelocity.SetX(m_afterEffects.GetPlanes().GetHorizontalPlaneVelocity(coords, newVelocity));
  }

  if (m_afterEffects.GetPlanes().IsVerticalPlaneVelocityActive())
  {
    newVelocity.SetY(m_afterEffects.GetPlanes().GetVerticalPlaneVelocity(coords, newVelocity));
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

auto ZoomVectorAfterEffects::SetRandomHypercosOverlayEffects() noexcept -> void
{
  switch (m_afterEffectsSettings.hypercosOverlay)
  {
    case HypercosOverlay::NONE:
      m_afterEffects.GetHypercos().SetDefaultParams();
      break;
    case HypercosOverlay::MODE0:
      m_afterEffects.GetHypercos().SetMode0RandomParams();
      break;
    case HypercosOverlay::MODE1:
      m_afterEffects.GetHypercos().SetMode1RandomParams();
      break;
    case HypercosOverlay::MODE2:
      m_afterEffects.GetHypercos().SetMode2RandomParams();
      break;
    case HypercosOverlay::MODE3:
      m_afterEffects.GetHypercos().SetMode3RandomParams();
      break;
    default:
      FailFast();
  }
}

inline auto ZoomVectorAfterEffects::SetRandomImageVelocityEffects() noexcept -> void
{
  if (not m_afterEffectsSettings.imageVelocityEffect)
  {
    return;
  }

  m_afterEffects.GetImageVelocity().SetRandomParams();
}

inline auto ZoomVectorAfterEffects::SetRandomNoiseSettings() noexcept -> void
{
  if (not m_afterEffectsSettings.noiseEffect)
  {
    return;
  }

  m_afterEffects.GetNoise().SetRandomParams();
}

inline auto ZoomVectorAfterEffects::SetRandomPlaneEffects() noexcept -> void
{
  if (not m_afterEffectsSettings.planeEffect)
  {
    return;
  }

  m_afterEffects.GetPlanes().SetRandomParams(m_zoomMidpoint, m_screenWidth);
}

inline auto ZoomVectorAfterEffects::SetRandomRotationSettings() noexcept -> void
{
  if (not m_afterEffectsSettings.rotationEffect)
  {
    return;
  }

  using Adj = RotationAdjustments::AdjustmentType;

  switch (m_rotationAdjustments.GetAdjustmentType())
  {
    case Adj::NONE:
      m_afterEffects.GetRotation().SetRandomParams();
      break;
    case Adj::INSTEAD_OF_RANDOM:
      m_afterEffects.GetRotation().ApplyAdjustments(m_rotationAdjustments);
      break;
    case Adj::AFTER_RANDOM:
      m_afterEffects.GetRotation().SetRandomParams();
      m_afterEffects.GetRotation().ApplyAdjustments(m_rotationAdjustments);
      break;
    default:
      FailFast();
  }
}

inline auto ZoomVectorAfterEffects::SetRandomTanEffects() noexcept -> void
{
  if (not m_afterEffectsSettings.tanEffect)
  {
    return;
  }

  m_afterEffects.GetTanEffect().SetRandomParams();
}

auto ZoomVectorAfterEffects::GetZoomEffectsNameValueParams() const noexcept -> NameValuePairs
{
  auto nameValuePairs = NameValuePairs{};

  MoveNameValuePairs(GetHypercosNameValueParams(), nameValuePairs);
  MoveNameValuePairs(GetImageVelocityNameValueParams(), nameValuePairs);
  MoveNameValuePairs(GetNoiseNameValueParams(), nameValuePairs);
  MoveNameValuePairs(GetPlaneNameValueParams(), nameValuePairs);
  MoveNameValuePairs(GetRotationNameValueParams(), nameValuePairs);
  MoveNameValuePairs(GetTanEffectNameValueParams(), nameValuePairs);

  return nameValuePairs;
}

inline auto ZoomVectorAfterEffects::GetHypercosNameValueParams() const noexcept -> NameValuePairs
{
  return m_afterEffects.GetHypercos().GetNameValueParams(PARAM_GROUP);
}

auto ZoomVectorAfterEffects::GetImageVelocityNameValueParams() const noexcept -> NameValuePairs
{
  auto nameValuePairs = NameValuePairs{
      GetPair(PARAM_GROUP, "imageVelocity", m_afterEffectsSettings.imageVelocityEffect)};
  if (m_afterEffectsSettings.imageVelocityEffect)
  {
    MoveNameValuePairs(m_afterEffects.GetImageVelocity().GetNameValueParams(PARAM_GROUP),
                       nameValuePairs);
  }
  return nameValuePairs;
}

auto ZoomVectorAfterEffects::GetNoiseNameValueParams() const noexcept -> NameValuePairs
{
  auto nameValuePairs =
      NameValuePairs{GetPair(PARAM_GROUP, "noiseEffect", m_afterEffectsSettings.noiseEffect)};
  if (m_afterEffectsSettings.noiseEffect)
  {
    MoveNameValuePairs(m_afterEffects.GetNoise().GetNameValueParams(PARAM_GROUP), nameValuePairs);
  }
  return nameValuePairs;
}

auto ZoomVectorAfterEffects::GetPlaneNameValueParams() const noexcept -> NameValuePairs
{
  auto nameValuePairs =
      NameValuePairs{GetPair(PARAM_GROUP, "planeEffect", m_afterEffectsSettings.planeEffect)};
  if (m_afterEffectsSettings.planeEffect)
  {
    MoveNameValuePairs(m_afterEffects.GetPlanes().GetNameValueParams(PARAM_GROUP), nameValuePairs);
  }
  return nameValuePairs;
}

auto ZoomVectorAfterEffects::GetRotationNameValueParams() const noexcept -> NameValuePairs
{
  auto nameValuePairs =
      NameValuePairs{GetPair(PARAM_GROUP, "rotation", m_afterEffectsSettings.rotationEffect)};
  if (m_afterEffectsSettings.rotationEffect)
  {
    MoveNameValuePairs(m_afterEffects.GetRotation().GetNameValueParams(PARAM_GROUP), nameValuePairs);
  }
  return nameValuePairs;
}

auto ZoomVectorAfterEffects::GetTanEffectNameValueParams() const noexcept -> NameValuePairs
{
  auto nameValuePairs =
      NameValuePairs{GetPair(PARAM_GROUP, "tanEffect", m_afterEffectsSettings.tanEffect)};
  if (m_afterEffectsSettings.tanEffect)
  {
    MoveNameValuePairs(m_afterEffects.GetTanEffect().GetNameValueParams(PARAM_GROUP), nameValuePairs);
  }
  return nameValuePairs;
}

} // namespace GOOM::FILTER_FX::AFTER_EFFECTS
