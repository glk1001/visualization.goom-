#pragma once

#include "filter_fx/after_effects/zoom_vector_after_effects.h"
#include "filter_fx/filter_settings.h"
#include "filter_fx/normalized_coords.h"
#include "filter_fx/speed_coefficients_effect.h"
#include "point2d.h"
#include "utils/name_value_pairs.h"

#include <functional>

namespace GOOM::UTILS::MATH
{
class IGoomRand;
}

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

class ZoomVectorEffects
{
public:
  using GetAfterEffectsFunc = std::function<auto(const GOOM::UTILS::MATH::IGoomRand& goomRand,
                                                 const std::string& resourcesDirectory)
                                                ->AFTER_EFFECTS::AfterEffects>;
  [[nodiscard]] static auto GetStandardAfterEffects(const GOOM::UTILS::MATH::IGoomRand& goomRand,
                                                    const std::string& resourcesDirectory)
      -> AFTER_EFFECTS::AfterEffects;

  ZoomVectorEffects(uint32_t screenWidth,
                    const std::string& resourcesDirectory,
                    const GOOM::UTILS::MATH::IGoomRand& goomRand,
                    const NormalizedCoordsConverter& normalizedCoordsConverter,
                    const GetAfterEffectsFunc& getAfterEffects) noexcept;

  auto SetFilterSettings(const ZoomFilterEffectsSettings& filterEffectsSettings) -> void;

  [[nodiscard]] auto GetSpeedCoeffVelocity(float sqDistFromZero,
                                           const NormalizedCoords& coords) const
      -> NormalizedCoords;
  [[nodiscard]] auto GetCleanedVelocity(const NormalizedCoords& velocity) const -> NormalizedCoords;

  [[nodiscard]] auto GetAfterEffectsVelocity(const NormalizedCoords& coords,
                                             float sqDistFromZero,
                                             const NormalizedCoords& velocity) const
      -> NormalizedCoords;

  [[nodiscard]] auto GetZoomEffectsNameValueParams() const -> GOOM::UTILS::NameValuePairs;

private:
  const NormalizedCoordsConverter& m_normalizedCoordsConverter;
  const ZoomFilterEffectsSettings* m_filterEffectsSettings{};
  AFTER_EFFECTS::ZoomVectorAfterEffects m_zoomVectorAfterEffects;

  [[nodiscard]] auto GetXYSpeedCoefficients(float sqDistFromZero,
                                            const NormalizedCoords& coords) const -> Point2dFlt;
  [[nodiscard]] auto GetBaseSpeedCoefficients() const -> Point2dFlt;
  [[nodiscard]] auto GetClampedSpeedCoeffs(const Point2dFlt& speedCoeffs) const -> Point2dFlt;
  [[nodiscard]] auto GetClampedSpeedCoeff(float speedCoeff) const -> float;
  [[nodiscard]] static auto GetMinVelocityVal(float velocityVal, float minNormalizedCoordVal)
      -> float;

  [[nodiscard]] auto GetSpeedCoefficientsNameValueParams() const -> GOOM::UTILS::NameValuePairs;
};

inline auto ZoomVectorEffects::GetSpeedCoeffVelocity(const float sqDistFromZero,
                                                     const NormalizedCoords& coords) const
    -> NormalizedCoords
{
  const auto xySpeedCoeffs      = GetXYSpeedCoefficients(sqDistFromZero, coords);
  const auto clampedSpeedCoeffs = GetClampedSpeedCoeffs(xySpeedCoeffs);

  return {clampedSpeedCoeffs.x * coords.GetX(), clampedSpeedCoeffs.y * coords.GetY()};
}

inline auto ZoomVectorEffects::GetXYSpeedCoefficients(const float sqDistFromZero,
                                                      const NormalizedCoords& coords) const
    -> Point2dFlt
{
  return m_filterEffectsSettings->speedCoefficientsEffect->GetSpeedCoefficients(
      coords, sqDistFromZero, GetBaseSpeedCoefficients());
  // Amulet 2
  // vx = X * tan(dist);
  // vy = Y * tan(dist);
}

inline auto ZoomVectorEffects::GetBaseSpeedCoefficients() const -> Point2dFlt
{
  static constexpr auto SPEED_COEFF_MULTIPLIER = 1.0F / 50.0F;
  const auto baseSpeedCoeff =
      SPEED_COEFF_MULTIPLIER * (1.0F + m_filterEffectsSettings->vitesse.GetRelativeSpeed());

  return {baseSpeedCoeff, baseSpeedCoeff};
}

inline auto ZoomVectorEffects::GetClampedSpeedCoeffs(const Point2dFlt& speedCoeffs) const
    -> Point2dFlt
{
  return {GetClampedSpeedCoeff(speedCoeffs.x), GetClampedSpeedCoeff(speedCoeffs.y)};
}

inline auto ZoomVectorEffects::GetClampedSpeedCoeff(const float speedCoeff) const -> float
{
  if (static constexpr auto MIN_SPEED_COEFF = -4.01F; speedCoeff < MIN_SPEED_COEFF)
  {
    return MIN_SPEED_COEFF;
  }
  if (speedCoeff > m_filterEffectsSettings->maxSpeedCoeff)
  {
    return m_filterEffectsSettings->maxSpeedCoeff;
  }
  return speedCoeff;
}

inline auto ZoomVectorEffects::GetAfterEffectsVelocity(const NormalizedCoords& coords,
                                                       const float sqDistFromZero,
                                                       const NormalizedCoords& velocity) const
    -> NormalizedCoords
{
  return m_zoomVectorAfterEffects.GetAfterEffectsVelocity(coords, sqDistFromZero, velocity);
}

inline auto ZoomVectorEffects::GetZoomEffectsNameValueParams() const -> GOOM::UTILS::NameValuePairs
{
  auto nameValuePairs = GOOM::UTILS::NameValuePairs{};

  GOOM::UTILS::MoveNameValuePairs(GetSpeedCoefficientsNameValueParams(), nameValuePairs);
  GOOM::UTILS::MoveNameValuePairs(m_zoomVectorAfterEffects.GetZoomEffectsNameValueParams(),
                                  nameValuePairs);

  return nameValuePairs;
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
