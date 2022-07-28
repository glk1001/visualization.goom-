#pragma once

#include "filter_fx/filter_settings.h"
#include "filter_fx/normalized_coords.h"
#include "filter_fx/speed_coefficients_effect.h"
#include "hypercos.h"
#include "image_velocity.h"
#include "noise.h"
#include "planes.h"
#include "point2d.h"
#include "rotation.h"
#include "tan_effect.h"
#include "utils/name_value_pairs.h"
#include "utils/propagate_const.h"

#include <functional>
#include <memory>

namespace GOOM
{

namespace UTILS::MATH
{
class IGoomRand;
}

namespace FILTER_FX::FILTER_EFFECTS
{

class ZoomVectorEffects
{
public:
  // NOLINTBEGIN
  struct TheExtraEffects
  {
    ~TheExtraEffects() noexcept;
    std::experimental::propagate_const<std::unique_ptr<Hypercos>> hypercos;
    std::experimental::propagate_const<std::unique_ptr<ImageVelocity>> imageVelocity;
    std::experimental::propagate_const<std::unique_ptr<Noise>> noise;
    std::experimental::propagate_const<std::unique_ptr<Planes>> planes;
    std::experimental::propagate_const<std::unique_ptr<Rotation>> rotation;
    std::experimental::propagate_const<std::unique_ptr<TanEffect>> tanEffect;
  };
  // NOLINTEND
  using GetTheEffectsFunc = std::function<auto(const std::string& resourcesDirectory,
                                               const UTILS::MATH::IGoomRand& goomRand)
                                              ->TheExtraEffects>;
  [[nodiscard]] static auto GetStandardExtraEffects(const std::string& resourcesDirectory,
                                                    const UTILS::MATH::IGoomRand& goomRand)
      -> TheExtraEffects;

  ZoomVectorEffects(uint32_t screenWidth,
                    const std::string& resourcesDirectory,
                    const UTILS::MATH::IGoomRand& goomRand,
                    const NormalizedCoordsConverter& normalizedCoordsConverter,
                    const GetTheEffectsFunc& getTheExtraEffects) noexcept;

  auto SetFilterSettings(const ZoomFilterEffectsSettings& filterEffectsSettings) -> void;

  [[nodiscard]] auto GetSpeedCoeffVelocity(float sqDistFromZero,
                                           const NormalizedCoords& coords) const
      -> NormalizedCoords;
  [[nodiscard]] auto GetCleanedVelocity(const NormalizedCoords& velocity) const -> NormalizedCoords;

  [[nodiscard]] auto IsHypercosOverlayActive() const -> bool;
  [[nodiscard]] auto GetHypercosVelocity(const NormalizedCoords& coords) const -> NormalizedCoords;

  [[nodiscard]] auto IsImageVelocityActive() const -> bool;
  [[nodiscard]] auto GetImageVelocity(const NormalizedCoords& coords) const -> NormalizedCoords;

  [[nodiscard]] auto IsNoiseActive() const -> bool;
  [[nodiscard]] auto GetNoiseVelocity() const -> NormalizedCoords;

  [[nodiscard]] auto IsHorizontalPlaneVelocityActive() const -> bool;
  [[nodiscard]] auto GetHorizontalPlaneVelocity(const NormalizedCoords& coords) const -> float;
  [[nodiscard]] auto IsVerticalPlaneVelocityActive() const -> bool;
  [[nodiscard]] auto GetVerticalPlaneVelocity(const NormalizedCoords& coords) const -> float;

  [[nodiscard]] auto IsRotationActive() const -> bool;
  [[nodiscard]] auto GetRotatedVelocity(const NormalizedCoords& velocity) const -> NormalizedCoords;

  [[nodiscard]] auto IsTanEffectActive() const -> bool;
  [[nodiscard]] auto GetTanEffectVelocity(float sqDistFromZero,
                                          const NormalizedCoords& velocity) const
      -> NormalizedCoords;

  static constexpr const char* PARAM_GROUP = "Zoom Effects";
  [[nodiscard]] auto GetZoomEffectsNameValueParams() const -> UTILS::NameValuePairs;

private:
  const uint32_t m_screenWidth;
  const NormalizedCoordsConverter& m_normalizedCoordsConverter;
  const ZoomFilterEffectsSettings* m_filterEffectsSettings{};
  TheExtraEffects m_theEffects;

  static constexpr float SPEED_COEFF_DENOMINATOR = 50.0F;
  static constexpr float MIN_SPEED_COEFF = -4.01F;

  auto SetRandomHypercosOverlayEffects() -> void;
  auto SetRandomImageVelocityEffects() -> void;
  auto SetRandomNoiseSettings() -> void;
  auto SetRandomPlaneEffects() -> void;
  auto SetRandomRotationSettings() -> void;
  auto SetRandomTanEffects() -> void;

  [[nodiscard]] auto GetMinVelocityVal(float velocityVal) const -> float;
  [[nodiscard]] auto GetXYSpeedCoefficients(float sqDistFromZero,
                                            const NormalizedCoords& coords) const -> Point2dFlt;
  [[nodiscard]] auto GetBaseSpeedCoefficients() const -> Point2dFlt;
  [[nodiscard]] auto GetClampedSpeedCoeffs(const Point2dFlt& speedCoeffs) const -> Point2dFlt;
  [[nodiscard]] auto GetClampedSpeedCoeff(float speedCoeff) const -> float;

  [[nodiscard]] auto GetHypercosNameValueParams() const -> UTILS::NameValuePairs;
  [[nodiscard]] auto GetImageVelocityNameValueParams() const -> UTILS::NameValuePairs;
  [[nodiscard]] auto GetNoiseNameValueParams() const -> UTILS::NameValuePairs;
  [[nodiscard]] auto GetPlaneNameValueParams() const -> UTILS::NameValuePairs;
  [[nodiscard]] auto GetRotationNameValueParams() const -> UTILS::NameValuePairs;
  [[nodiscard]] auto GetSpeedCoefficientsNameValueParams() const -> UTILS::NameValuePairs;
  [[nodiscard]] auto GetTanEffectNameValueParams() const -> UTILS::NameValuePairs;
};

inline auto ZoomVectorEffects::GetSpeedCoeffVelocity(const float sqDistFromZero,
                                                     const NormalizedCoords& coords) const
    -> NormalizedCoords
{
  const auto speedCoeffs = GetClampedSpeedCoeffs(GetXYSpeedCoefficients(sqDistFromZero, coords));
  return {speedCoeffs.x * coords.GetX(), speedCoeffs.y * coords.GetY()};
}

inline auto ZoomVectorEffects::GetXYSpeedCoefficients(const float sqDistFromZero,
                                                      const NormalizedCoords& coords) const
    -> Point2dFlt
{
  return m_filterEffectsSettings->speedCoefficientsEffect->GetSpeedCoefficients(
      GetBaseSpeedCoefficients(), sqDistFromZero, coords);
  // Amulet 2
  // vx = X * tan(dist);
  // vy = Y * tan(dist);
}

inline auto ZoomVectorEffects::GetBaseSpeedCoefficients() const -> Point2dFlt
{
  const auto speedCoeff =
      (1.0F + m_filterEffectsSettings->vitesse.GetRelativeSpeed()) / SPEED_COEFF_DENOMINATOR;
  return {speedCoeff, speedCoeff};
}

inline auto ZoomVectorEffects::GetClampedSpeedCoeffs(const Point2dFlt& speedCoeffs) const
    -> Point2dFlt
{
  return {GetClampedSpeedCoeff(speedCoeffs.x), GetClampedSpeedCoeff(speedCoeffs.y)};
}

inline auto ZoomVectorEffects::GetClampedSpeedCoeff(const float speedCoeff) const -> float
{
  if (speedCoeff < MIN_SPEED_COEFF)
  {
    return MIN_SPEED_COEFF;
  }
  if (speedCoeff > m_filterEffectsSettings->maxSpeedCoeff)
  {
    return m_filterEffectsSettings->maxSpeedCoeff;
  }
  return speedCoeff;
}

inline auto ZoomVectorEffects::IsHypercosOverlayActive() const -> bool
{
  return m_filterEffectsSettings->hypercosOverlay != HypercosOverlay::NONE;
}

inline auto ZoomVectorEffects::GetHypercosVelocity(const NormalizedCoords& coords) const
    -> NormalizedCoords
{
  return m_theEffects.hypercos->GetVelocity(coords);
}

inline auto ZoomVectorEffects::IsImageVelocityActive() const -> bool
{
  return m_filterEffectsSettings->imageVelocityEffect;
}

inline auto ZoomVectorEffects::GetImageVelocity(const NormalizedCoords& coords) const
    -> NormalizedCoords
{
  return NormalizedCoords{m_theEffects.imageVelocity->GetVelocity(coords)};
}

inline auto ZoomVectorEffects::IsNoiseActive() const -> bool
{
  return m_filterEffectsSettings->noiseEffect;
}

inline auto ZoomVectorEffects::GetNoiseVelocity() const -> NormalizedCoords
{
  return m_theEffects.noise->GetVelocity();
}

inline auto ZoomVectorEffects::IsHorizontalPlaneVelocityActive() const -> bool
{
  return m_theEffects.planes->IsHorizontalPlaneVelocityActive();
}

inline auto ZoomVectorEffects::GetHorizontalPlaneVelocity(const NormalizedCoords& coords) const
    -> float
{
  return m_theEffects.planes->GetHorizontalPlaneVelocity(coords);
}

inline auto ZoomVectorEffects::IsVerticalPlaneVelocityActive() const -> bool
{
  return m_theEffects.planes->IsVerticalPlaneVelocityActive();
}

inline auto ZoomVectorEffects::GetVerticalPlaneVelocity(const NormalizedCoords& coords) const
    -> float
{
  return m_theEffects.planes->GetVerticalPlaneVelocity(coords);
}

inline auto ZoomVectorEffects::IsRotationActive() const -> bool
{
  return m_filterEffectsSettings->rotationEffect;
}

inline auto ZoomVectorEffects::GetRotatedVelocity(const NormalizedCoords& velocity) const
    -> NormalizedCoords
{
  return m_theEffects.rotation->GetVelocity(velocity);
}

inline auto ZoomVectorEffects::IsTanEffectActive() const -> bool
{
  return m_filterEffectsSettings->tanEffect;
}

inline auto ZoomVectorEffects::GetTanEffectVelocity(const float sqDistFromZero,
                                                    const NormalizedCoords& velocity) const
    -> NormalizedCoords
{
  return m_theEffects.tanEffect->GetVelocity(sqDistFromZero, velocity);
}

} // namespace FILTER_FX::FILTER_EFFECTS
} // namespace GOOM
