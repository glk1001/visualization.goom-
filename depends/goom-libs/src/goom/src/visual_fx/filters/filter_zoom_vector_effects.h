#pragma once

#include "filter_settings.h"
#include "hypercos.h"
#include "image_velocity.h"
#include "noise.h"
#include "normalized_coords.h"
#include "planes.h"
#include "rotation.h"
#include "speed_coefficients_effect.h"
#include "tan_effect.h"
#include "utils/name_value_pairs.h"
#include "v2d.h"

#include <functional>
#include <memory>

namespace GOOM
{

namespace UTILS
{
class IGoomRand;
} // namespace UTILS

namespace VISUAL_FX
{
namespace FILTERS
{

class ZoomVectorEffects
{
public:
  struct TheExtraEffects
  {
    std::unique_ptr<ImageVelocity> imageVelocity;
    std::unique_ptr<Noise> noise;
    std::unique_ptr<Hypercos> hypercos;
    std::unique_ptr<Planes> planes;
    std::unique_ptr<TanEffect> tanEffect;
  };
  using GetTheEffectsFunc =
      std::function<auto(const std::string& resourcesDirectory, const UTILS::IGoomRand& goomRand)
                        ->TheExtraEffects>;
  [[nodiscard]] static auto GetStandardExtraEffects(const std::string& resourcesDirectory,
                                                    const UTILS::IGoomRand& goomRand)
      -> TheExtraEffects;

  ZoomVectorEffects(uint32_t screenWidth,
                    const std::string& resourcesDirectory,
                    const UTILS::IGoomRand& goomRand,
                    const GetTheEffectsFunc& getTheExtraEffects) noexcept;

  void SetFilterSettings(const ZoomFilterEffectsSettings& filterEffectsSettings);

  [[nodiscard]] auto GetSpeedCoeffVelocity(float sqDistFromZero,
                                           const NormalizedCoords& coords) const
      -> NormalizedCoords;
  [[nodiscard]] static auto GetCleanedVelocity(const NormalizedCoords& velocity)
      -> NormalizedCoords;

  [[nodiscard]] auto IsImageVelocityActive() const -> bool;
  [[nodiscard]] auto GetImageVelocity(const NormalizedCoords& coords) const -> NormalizedCoords;

  [[nodiscard]] auto IsRotateActive() const -> bool;
  [[nodiscard]] auto GetRotatedVelocity(const NormalizedCoords& velocity) const -> NormalizedCoords;

  [[nodiscard]] auto IsNoiseActive() const -> bool;
  [[nodiscard]] auto GetNoiseVelocity() const -> NormalizedCoords;

  [[nodiscard]] auto IsTanEffectActive() const -> bool;
  [[nodiscard]] auto GetTanEffectVelocity(float sqDistFromZero,
                                          const NormalizedCoords& velocity) const
      -> NormalizedCoords;

  [[nodiscard]] auto IsHypercosOverlayActive() const -> bool;
  [[nodiscard]] auto GetHypercosVelocity(const NormalizedCoords& coords) const -> NormalizedCoords;

  [[nodiscard]] auto IsHorizontalPlaneVelocityActive() const -> bool;
  [[nodiscard]] auto GetHorizontalPlaneVelocity(const NormalizedCoords& coords) const -> float;
  [[nodiscard]] auto IsVerticalPlaneVelocityActive() const -> bool;
  [[nodiscard]] auto GetVerticalPlaneVelocity(const NormalizedCoords& coords) const -> float;

  static constexpr const char* PARAM_GROUP = "Zoom Effects";
  [[nodiscard]] auto GetZoomEffectsNameValueParams() const -> UTILS::NameValuePairs;

private:
  const uint32_t m_screenWidth;
  const ZoomFilterEffectsSettings* m_filterEffectsSettings{};
  TheExtraEffects m_theEffects;

  static constexpr float SPEED_COEFF_DENOMINATOR = 50.0F;
  static constexpr float MIN_SPEED_COEFF = -4.01F;

  void SetNoiseSettings();
  void SetRandomImageVelocityEffects();
  void SetRandomHypercosOverlayEffects();
  void SetRandomPlaneEffects();
  void SetRandomTanEffects();

  [[nodiscard]] static auto GetMinVelocityVal(float velocityVal) -> float;
  [[nodiscard]] auto GetXYSpeedCoefficients(float sqDistFromZero,
                                            const NormalizedCoords& coords) const -> V2dFlt;
  [[nodiscard]] auto GetBaseSpeedCoefficients() const -> V2dFlt;
  [[nodiscard]] auto GetClampedSpeedCoeffs(const V2dFlt& speedCoeffs) const -> V2dFlt;
  [[nodiscard]] auto GetClampedSpeedCoeff(float speedCoeff) const -> float;

  [[nodiscard]] auto GetSpeedCoefficientsNameValueParams() const -> UTILS::NameValuePairs;
  [[nodiscard]] auto GetImageVelocityNameValueParams() const -> UTILS::NameValuePairs;
  [[nodiscard]] auto GetRotateNameValueParams() const -> UTILS::NameValuePairs;
  [[nodiscard]] auto GetNoiseNameValueParams() const -> UTILS::NameValuePairs;
  [[nodiscard]] auto GetTanEffectNameValueParams() const -> UTILS::NameValuePairs;
  [[nodiscard]] auto GetHypercosNameValueParams() const -> UTILS::NameValuePairs;
  [[nodiscard]] auto GetPlaneNameValueParams() const -> UTILS::NameValuePairs;
};

inline auto ZoomVectorEffects::GetSpeedCoeffVelocity(const float sqDistFromZero,
                                                     const NormalizedCoords& coords) const
    -> NormalizedCoords
{
  const V2dFlt speedCoeffs = GetClampedSpeedCoeffs(GetXYSpeedCoefficients(sqDistFromZero, coords));
  return {speedCoeffs.x * coords.GetX(), speedCoeffs.y * coords.GetY()};
}

inline auto ZoomVectorEffects::GetXYSpeedCoefficients(const float sqDistFromZero,
                                                      const NormalizedCoords& coords) const
    -> V2dFlt
{
  return m_filterEffectsSettings->speedCoefficientsEffect->GetSpeedCoefficients(
      GetBaseSpeedCoefficients(), sqDistFromZero, coords);
  // Amulet 2
  // vx = X * tan(dist);
  // vy = Y * tan(dist);
}

inline auto ZoomVectorEffects::GetBaseSpeedCoefficients() const -> V2dFlt
{
  const float speedCoeff =
      (1.0F + m_filterEffectsSettings->vitesse.GetRelativeSpeed()) / SPEED_COEFF_DENOMINATOR;
  return {speedCoeff, speedCoeff};
}

inline auto ZoomVectorEffects::GetClampedSpeedCoeffs(const V2dFlt& speedCoeffs) const -> V2dFlt
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

inline auto ZoomVectorEffects::IsImageVelocityActive() const -> bool
{
  return m_filterEffectsSettings->imageVelocityEffect;
}

inline auto ZoomVectorEffects::GetImageVelocity(const NormalizedCoords& coords) const
    -> NormalizedCoords
{
  return NormalizedCoords{m_theEffects.imageVelocity->GetVelocity(coords)};
}

inline auto ZoomVectorEffects::IsRotateActive() const -> bool
{
  return m_filterEffectsSettings->rotation->IsActive();
}

inline auto ZoomVectorEffects::GetRotatedVelocity(const NormalizedCoords& velocity) const
    -> NormalizedCoords
{
  return m_filterEffectsSettings->rotation->GetVelocity(velocity);
}

inline auto ZoomVectorEffects::IsNoiseActive() const -> bool
{
  return m_filterEffectsSettings->noiseEffect;
}

inline auto ZoomVectorEffects::GetNoiseVelocity() const -> NormalizedCoords
{
  return m_theEffects.noise->GetVelocity();
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

inline auto ZoomVectorEffects::IsHypercosOverlayActive() const -> bool
{
  return m_filterEffectsSettings->hypercosOverlay != HypercosOverlay::NONE;
}

inline auto ZoomVectorEffects::GetHypercosVelocity(const NormalizedCoords& coords) const
    -> NormalizedCoords
{
  return m_theEffects.hypercos->GetVelocity(coords);
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

} // namespace FILTERS
} // namespace VISUAL_FX
} // namespace GOOM
