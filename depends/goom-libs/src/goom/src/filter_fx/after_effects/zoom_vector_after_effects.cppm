module;

#include "goom/point2d.h"

#include <cstdint>

export module Goom.FilterFx.AfterEffects.ZoomVectorAfterEffects;

import Goom.FilterFx.AfterEffects.TheEffects.Rotation;
import Goom.FilterFx.AfterEffects.AfterEffects;
import Goom.FilterFx.AfterEffects.AfterEffectsStates;
import Goom.FilterFx.NormalizedCoords;
import Goom.Utils.NameValuePairs;

export namespace GOOM::FILTER_FX::AFTER_EFFECTS
{

class ZoomVectorAfterEffects
{
public:
  ZoomVectorAfterEffects(uint32_t screenWidth, AfterEffects&& afterEffects) noexcept;

  auto SetAfterEffectsSettings(const AfterEffectsStates::AfterEffectsSettings& afterEffectsSettings,
                               const Point2dInt& zoomMidpoint) noexcept -> void;

  [[nodiscard]] auto GetAfterEffectsVelocity(const NormalizedCoords& coords,
                                             float sqDistFromZero,
                                             const NormalizedCoords& zoomVelocity) const noexcept
      -> NormalizedCoords;

  static constexpr auto* PARAM_GROUP = "After Effects";
  [[nodiscard]] auto GetZoomEffectsNameValueParams() const noexcept -> UTILS::NameValuePairs;

private:
  uint32_t m_screenWidth;
  AfterEffects m_afterEffects;
  AfterEffectsStates::AfterEffectsSettings m_afterEffectsSettings{};
  RotationAdjustments m_rotationAdjustments{};
  Point2dInt m_zoomMidpoint{};

  auto SetRandomHypercosOverlayEffects() noexcept -> void;
  [[nodiscard]] auto GetHypercosNameValueParams() const noexcept -> UTILS::NameValuePairs;

  auto SetRandomImageVelocityEffects() noexcept -> void;
  [[nodiscard]] auto GetImageVelocityNameValueParams() const noexcept -> UTILS::NameValuePairs;

  auto SetRandomNoiseSettings() noexcept -> void;
  [[nodiscard]] auto GetNoiseNameValueParams() const noexcept -> UTILS::NameValuePairs;

  auto SetRandomPlaneEffects() noexcept -> void;
  [[nodiscard]] auto GetPlaneNameValueParams() const noexcept -> UTILS::NameValuePairs;

  auto SetRandomRotationSettings() noexcept -> void;
  [[nodiscard]] auto GetRotationNameValueParams() const noexcept -> UTILS::NameValuePairs;

  auto SetRandomTanEffects() noexcept -> void;
  [[nodiscard]] auto GetTanEffectNameValueParams() const noexcept -> UTILS::NameValuePairs;

  auto SetRandomXYLerpEffects() noexcept -> void;
  [[nodiscard]] auto GetXYLerpEffectNameValueParams() const noexcept -> UTILS::NameValuePairs;
};

} // namespace GOOM::FILTER_FX::AFTER_EFFECTS
