module;

#include <cstdint>
#include <utility>

module Goom.FilterFx.AfterEffects.ZoomVectorAfterEffects;

import Goom.FilterFx.AfterEffects.TheEffects.Hypercos;
import Goom.FilterFx.AfterEffects.TheEffects.ImageVelocity;
import Goom.FilterFx.AfterEffects.TheEffects.Noise;
import Goom.FilterFx.AfterEffects.TheEffects.Planes;
import Goom.FilterFx.AfterEffects.TheEffects.Rotation;
import Goom.FilterFx.AfterEffects.TheEffects.TanEffect;
import Goom.FilterFx.AfterEffects.TheEffects.XYLerpEffect;
import Goom.Utils.Math.GoomRand;
import Goom.FilterFx.AfterEffects.AfterEffects;
import Goom.FilterFx.AfterEffects.AfterEffectsStates;
import Goom.FilterFx.AfterEffects.AfterEffectsTypes;
import Goom.FilterFx.NormalizedCoords;
import Goom.Utils.NameValuePairs;
import Goom.Lib.Point2d;

namespace GOOM::FILTER_FX::AFTER_EFFECTS
{

using UTILS::GetPair;
using UTILS::MoveNameValuePairs;
using UTILS::NameValuePairs;

ZoomVectorAfterEffects::ZoomVectorAfterEffects(const uint32_t screenWidth,
                                               AfterEffects&& afterEffects) noexcept
  : m_screenWidth{screenWidth}, m_afterEffects{std::move(afterEffects)}
{
}

auto ZoomVectorAfterEffects::SetAfterEffectsSettings(
    const AfterEffectsStates::AfterEffectsSettings& afterEffectsSettings,
    const Point2dInt& zoomMidpoint) noexcept -> void
{
  m_afterEffectsSettings = afterEffectsSettings;
  m_rotationAdjustments  = afterEffectsSettings.rotationAdjustments; // TODO(glk) Fix this??
  m_zoomMidpoint         = zoomMidpoint;

  SetRandomHypercosOverlayEffects();
  SetRandomImageVelocityEffects();
  SetRandomNoiseSettings();
  SetRandomPlaneEffects();
  SetRandomRotationSettings();
  SetRandomTanEffects();
  SetRandomXYLerpEffects();
}

auto ZoomVectorAfterEffects::GetAfterEffectsVelocity(
    const NormalizedCoords& coords,
    const float sqDistFromZero,
    const NormalizedCoords& zoomVelocity) const noexcept -> NormalizedCoords
{
  auto newVelocity = zoomVelocity;

  if (m_afterEffectsSettings.isActive[AfterEffectsTypes::IMAGE_VELOCITY])
  {
    newVelocity =
        m_afterEffects.GetImageVelocity().GetVelocity({.coords = coords, .velocity = newVelocity});
  }

  if (m_afterEffectsSettings.isActive[AfterEffectsTypes::XY_LERP_EFFECT])
  {
    newVelocity = m_afterEffects.GetXYLerpEffect().GetVelocity(sqDistFromZero, newVelocity);
  }

  if (m_afterEffectsSettings.isActive[AfterEffectsTypes::ROTATION])
  {
    newVelocity = m_afterEffects.GetRotation().GetVelocity(newVelocity);
  }

  if (m_afterEffectsSettings.isActive[AfterEffectsTypes::TAN_EFFECT])
  {
    newVelocity = m_afterEffects.GetTanEffect().GetVelocity(sqDistFromZero, newVelocity);
  }

  if (m_afterEffectsSettings.isActive[AfterEffectsTypes::NOISE])
  {
    newVelocity = m_afterEffects.GetNoise().GetVelocity(newVelocity);
  }

  if (m_afterEffectsSettings.hypercosOverlayMode != HypercosOverlayMode::NONE)
  {
    newVelocity = m_afterEffects.GetHypercos().GetVelocity(coords, newVelocity);
  }

  if (m_afterEffects.GetPlanes().IsHorizontalPlaneVelocityActive())
  {
    newVelocity.SetX(m_afterEffects.GetPlanes().GetHorizontalPlaneVelocity(
        {.coords = coords, .velocity = newVelocity}));
  }

  if (m_afterEffects.GetPlanes().IsVerticalPlaneVelocityActive())
  {
    newVelocity.SetY(m_afterEffects.GetPlanes().GetVerticalPlaneVelocity(
        {.coords = coords, .velocity = newVelocity}));
  }

  /* TODO(glk): Water Mode */
  //    if (data->waveEffect)

  /**
  if (m_filterSettings->mode == ZoomFilterMode::NORMAL_MODE)
    if (ProbabilityOfMInN(1, 2))
      velocity = {-2.0F * xNormalized + velocity.x, -2.0F * yNormalized + velocity.y};
  **/

  return newVelocity - zoomVelocity;
}

auto ZoomVectorAfterEffects::SetRandomHypercosOverlayEffects() noexcept -> void
{
  switch (m_afterEffectsSettings.hypercosOverlayMode)
  {
    case HypercosOverlayMode::NONE:
      m_afterEffects.GetHypercos().SetDefaultParams();
      break;
    case HypercosOverlayMode::MODE0:
      m_afterEffects.GetHypercos().SetMode0RandomParams();
      break;
    case HypercosOverlayMode::MODE1:
      m_afterEffects.GetHypercos().SetMode1RandomParams();
      break;
    case HypercosOverlayMode::MODE2:
      m_afterEffects.GetHypercos().SetMode2RandomParams();
      break;
    case HypercosOverlayMode::MODE3:
      m_afterEffects.GetHypercos().SetMode3RandomParams();
      break;
  }
}

inline auto ZoomVectorAfterEffects::SetRandomImageVelocityEffects() noexcept -> void
{
  if (not m_afterEffectsSettings.isActive[AfterEffectsTypes::IMAGE_VELOCITY])
  {
    return;
  }

  m_afterEffects.GetImageVelocity().SetRandomParams();
}

inline auto ZoomVectorAfterEffects::SetRandomNoiseSettings() noexcept -> void
{
  if (not m_afterEffectsSettings.isActive[AfterEffectsTypes::NOISE])
  {
    return;
  }

  m_afterEffects.GetNoise().SetRandomParams();
}

inline auto ZoomVectorAfterEffects::SetRandomPlaneEffects() noexcept -> void
{
  if (not m_afterEffectsSettings.isActive[AfterEffectsTypes::PLANES])
  {
    return;
  }

  m_afterEffects.GetPlanes().SetRandomParams(m_zoomMidpoint, m_screenWidth);
}

inline auto ZoomVectorAfterEffects::SetRandomRotationSettings() noexcept -> void
{
  if (not m_afterEffectsSettings.isActive[AfterEffectsTypes::ROTATION])
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
  }
}

inline auto ZoomVectorAfterEffects::SetRandomTanEffects() noexcept -> void
{
  if (not m_afterEffectsSettings.isActive[AfterEffectsTypes::TAN_EFFECT])
  {
    return;
  }

  m_afterEffects.GetTanEffect().SetRandomParams();
}

inline auto ZoomVectorAfterEffects::SetRandomXYLerpEffects() noexcept -> void
{
  if (not m_afterEffectsSettings.isActive[AfterEffectsTypes::XY_LERP_EFFECT])
  {
    return;
  }

  m_afterEffects.GetXYLerpEffect().SetRandomParams();
}

auto ZoomVectorAfterEffects::GetAfterEffectsNameValueParams() const noexcept -> NameValuePairs
{
  auto nameValuePairs = NameValuePairs{};

  MoveNameValuePairs(GetHypercosNameValueParams(), nameValuePairs);
  MoveNameValuePairs(GetImageVelocityNameValueParams(), nameValuePairs);
  MoveNameValuePairs(GetNoiseNameValueParams(), nameValuePairs);
  MoveNameValuePairs(GetPlaneNameValueParams(), nameValuePairs);
  MoveNameValuePairs(GetRotationNameValueParams(), nameValuePairs);
  MoveNameValuePairs(GetTanEffectNameValueParams(), nameValuePairs);
  MoveNameValuePairs(GetXYLerpEffectNameValueParams(), nameValuePairs);

  return nameValuePairs;
}

inline auto ZoomVectorAfterEffects::GetHypercosNameValueParams() const noexcept -> NameValuePairs
{
  return m_afterEffects.GetHypercos().GetNameValueParams(PARAM_GROUP);
}

auto ZoomVectorAfterEffects::GetImageVelocityNameValueParams() const noexcept -> NameValuePairs
{
  const auto isActive = m_afterEffectsSettings.isActive[AfterEffectsTypes::IMAGE_VELOCITY];
  auto nameValuePairs = NameValuePairs{GetPair(PARAM_GROUP, "imageVelocity", isActive)};

  if (isActive)
  {
    MoveNameValuePairs(m_afterEffects.GetImageVelocity().GetNameValueParams(PARAM_GROUP),
                       nameValuePairs);
  }

  return nameValuePairs;
}

auto ZoomVectorAfterEffects::GetNoiseNameValueParams() const noexcept -> NameValuePairs
{
  const auto isActive = m_afterEffectsSettings.isActive[AfterEffectsTypes::NOISE];
  auto nameValuePairs = NameValuePairs{GetPair(PARAM_GROUP, "noiseEffect", isActive)};

  if (isActive)
  {
    MoveNameValuePairs(m_afterEffects.GetNoise().GetNameValueParams(PARAM_GROUP), nameValuePairs);
  }

  return nameValuePairs;
}

auto ZoomVectorAfterEffects::GetPlaneNameValueParams() const noexcept -> NameValuePairs
{
  const auto isActive = m_afterEffectsSettings.isActive[AfterEffectsTypes::PLANES];
  auto nameValuePairs = NameValuePairs{GetPair(PARAM_GROUP, "planeEffect", isActive)};

  if (isActive)
  {
    MoveNameValuePairs(m_afterEffects.GetPlanes().GetNameValueParams(PARAM_GROUP), nameValuePairs);
  }

  return nameValuePairs;
}

auto ZoomVectorAfterEffects::GetRotationNameValueParams() const noexcept -> NameValuePairs
{
  const auto isActive = m_afterEffectsSettings.isActive[AfterEffectsTypes::ROTATION];
  auto nameValuePairs = NameValuePairs{GetPair(PARAM_GROUP, "rotation", isActive)};

  if (isActive)
  {
    MoveNameValuePairs(m_afterEffects.GetRotation().GetNameValueParams(PARAM_GROUP),
                       nameValuePairs);
  }

  return nameValuePairs;
}

auto ZoomVectorAfterEffects::GetTanEffectNameValueParams() const noexcept -> NameValuePairs
{
  const auto isActive = m_afterEffectsSettings.isActive[AfterEffectsTypes::TAN_EFFECT];
  auto nameValuePairs = NameValuePairs{GetPair(PARAM_GROUP, "tanEffect", isActive)};

  if (isActive)
  {
    MoveNameValuePairs(m_afterEffects.GetTanEffect().GetNameValueParams(PARAM_GROUP),
                       nameValuePairs);
  }

  return nameValuePairs;
}

auto ZoomVectorAfterEffects::GetXYLerpEffectNameValueParams() const noexcept
    -> UTILS::NameValuePairs
{
  const auto isActive = m_afterEffectsSettings.isActive[AfterEffectsTypes::XY_LERP_EFFECT];
  auto nameValuePairs = NameValuePairs{GetPair(PARAM_GROUP, "xyLerpEffect", isActive)};

  if (isActive)
  {
    MoveNameValuePairs(m_afterEffects.GetXYLerpEffect().GetNameValueParams(PARAM_GROUP),
                       nameValuePairs);
  }

  return nameValuePairs;
}

} // namespace GOOM::FILTER_FX::AFTER_EFFECTS
