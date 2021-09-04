#ifndef VISUALIZATION_GOOM_FILTER_ZOOM_VECTOR_EFFECTS_H
#define VISUALIZATION_GOOM_FILTER_ZOOM_VECTOR_EFFECTS_H

#include "filter_normalized_coords.h"
#include "filter_settings.h"
#include "filter_speed_coefficients_effect.h"
#include "goomutils/mathutils.h"
#include "v2d.h"

#include <cmath>
#include <memory>
#include <string>
#include <vector>

#if __cplusplus <= 201402L
namespace GOOM
{

namespace FILTERS
{
#else
namespace GOOM::FILTERS
{
#endif

class Hypercos;
class Planes;

class ZoomVectorEffects
{
public:
  ZoomVectorEffects(uint32_t screenWidth, const std::string& resourcesDirectory) noexcept;
  ZoomVectorEffects(const ZoomVectorEffects&) noexcept = delete;
  ZoomVectorEffects(ZoomVectorEffects&&) noexcept = delete;
  ~ZoomVectorEffects() noexcept;
  auto operator=(const ZoomVectorEffects&) -> ZoomVectorEffects& = delete;
  auto operator=(ZoomVectorEffects&&) -> ZoomVectorEffects& = delete;

  void SetFilterSettings(const ZoomFilterSettings& filterSettings);

  void SetMaxSpeedCoeff(float val);

  [[nodiscard]] auto GetSpeedCoeffVelocity(float sqDistFromZero,
                                           const NormalizedCoords& coords) const
      -> NormalizedCoords;
  [[nodiscard]] static auto GetCleanedVelocity(const NormalizedCoords& velocity)
      -> NormalizedCoords;

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

private:
  const uint32_t m_screenWidth;
  const std::string m_resourcesDirectory;
  const ZoomFilterSettings* m_filterSettings{};

  // For noise amplitude, take the reciprocal of these.
  static constexpr float NOISE_MIN = 40.0F;
  static constexpr float NOISE_MAX = 120.0F;

  static constexpr float SPEED_COEFF_DENOMINATOR = 50.0F;
  static constexpr float MIN_SPEED_COEFF = -4.01F;
  static constexpr float DEFAULT_MAX_SPEED_COEFF = +2.01F;
  float m_maxSpeedCoeff = DEFAULT_MAX_SPEED_COEFF;

  const std::unique_ptr<Hypercos> m_hypercos;
  const std::unique_ptr<Planes> m_planes;

  void SetHypercosOverlaySettings();
  void SetRandomPlaneEffects();

  [[nodiscard]] static auto GetMinVelocityVal(float velocityVal) -> float;
  [[nodiscard]] auto GetXYSpeedCoefficients(float sqDistFromZero,
                                            const NormalizedCoords& coords) const -> V2dFlt;
  [[nodiscard]] auto GetBaseSpeedCoefficients() const -> V2dFlt;
  [[nodiscard]] auto GetClampedSpeedCoeffs(const V2dFlt& speedCoeffs) const -> V2dFlt;
  [[nodiscard]] auto GetClampedSpeedCoeff(float speedCoeff) const -> float;
};

inline void ZoomVectorEffects::SetMaxSpeedCoeff(const float val)
{
  m_maxSpeedCoeff = val;
}

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
  return m_filterSettings->speedCoefficientsEffect->GetSpeedCoefficients(GetBaseSpeedCoefficients(),
                                                                         sqDistFromZero, coords);
  // Amulet 2
  // vx = X * tan(dist);
  // vy = Y * tan(dist);
}

inline auto ZoomVectorEffects::GetBaseSpeedCoefficients() const -> V2dFlt
{
  const float speedCoeff =
      (1.0F + m_filterSettings->vitesse.GetRelativeSpeed()) / SPEED_COEFF_DENOMINATOR;
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
  if (speedCoeff > m_maxSpeedCoeff)
  {
    return m_maxSpeedCoeff;
  }
  return speedCoeff;
}

inline auto ZoomVectorEffects::IsRotateActive() const -> bool
{
  return std::fabs(m_filterSettings->rotateSpeed) > UTILS::SMALL_FLOAT;
}

inline auto ZoomVectorEffects::IsNoiseActive() const -> bool
{
  return m_filterSettings->noisify;
}

inline auto ZoomVectorEffects::IsTanEffectActive() const -> bool
{
  return m_filterSettings->tanEffect;
}

inline auto ZoomVectorEffects::IsHypercosOverlayActive() const -> bool
{
  return m_filterSettings->hypercosOverlay != HypercosOverlay::NONE;
}

#if __cplusplus <= 201402L
} // namespace FILTERS
} // namespace GOOM
#else
} // namespace GOOM::FILTERS
#endif

#endif //VISUALIZATION_GOOM_FILTER_ZOOM_VECTOR_EFFECTS_H
