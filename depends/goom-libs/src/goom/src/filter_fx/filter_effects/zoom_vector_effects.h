#pragma once

#include "filter_fx/after_effects/zoom_vector_after_effects.h"
#include "filter_fx/filter_settings.h"
#include "filter_fx/normalized_coords.h"
#include "filter_fx/zoom_in_coefficients_effect.h"
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

  static constexpr float RAW_BASE_ZOOM_IN_COEFF_FACTOR = 0.02F;
  auto SetFilterSettings(const ZoomFilterEffectsSettings& filterEffectsSettings) -> void;

  [[nodiscard]] auto GetZoomInCoefficients(const NormalizedCoords& coords,
                                           float sqDistFromZero) const -> Point2dFlt;
  [[nodiscard]] auto GetCleanedCoords(const NormalizedCoords& coords) const -> NormalizedCoords;

  [[nodiscard]] auto GetAfterEffectsVelocity(const NormalizedCoords& coords,
                                             float sqDistFromZero,
                                             const NormalizedCoords& zoomInVelocity) const
      -> NormalizedCoords;

  [[nodiscard]] auto GetZoomEffectsNameValueParams() const -> GOOM::UTILS::NameValuePairs;

private:
  const NormalizedCoordsConverter& m_normalizedCoordsConverter;
  const ZoomFilterEffectsSettings* m_filterEffectsSettings{};
  AFTER_EFFECTS::ZoomVectorAfterEffects m_zoomVectorAfterEffects;
  float m_baseZoomInCoeffFactor = RAW_BASE_ZOOM_IN_COEFF_FACTOR;
  auto SetBaseZoomInCoeffFactor(float multiplier) noexcept -> void;

  [[nodiscard]] auto GetBaseZoomInCoeffs() const -> Point2dFlt;
  [[nodiscard]] auto GetClampedZoomInCoeffs(const Point2dFlt& zoomCoeffs) const -> Point2dFlt;
  [[nodiscard]] auto GetClampedZoomInCoeff(float zoomCoeff) const -> float;
  [[nodiscard]] static auto GetMinCoordVal(float coordVal, float minNormalizedCoordVal) -> float;

  [[nodiscard]] auto GetZoomInCoeffsNameValueParams() const -> GOOM::UTILS::NameValuePairs;
};

inline auto ZoomVectorEffects::GetZoomInCoefficients(const NormalizedCoords& coords,
                                                     const float sqDistFromZero) const -> Point2dFlt
{
  const auto zoomCoeffs = m_filterEffectsSettings->zoomInCoefficientsEffect->GetZoomInCoefficients(
      coords, sqDistFromZero, GetBaseZoomInCoeffs());
  // Amulet 2
  // vx = X * tan(dist);
  // vy = Y * tan(dist);
  return GetClampedZoomInCoeffs(zoomCoeffs);
}

inline auto ZoomVectorEffects::SetBaseZoomInCoeffFactor(const float multiplier) noexcept -> void
{
  Expects(multiplier > 0.0F);
  m_baseZoomInCoeffFactor = multiplier * RAW_BASE_ZOOM_IN_COEFF_FACTOR;
}

inline auto ZoomVectorEffects::GetBaseZoomInCoeffs() const -> Point2dFlt
{
  const auto baseZoomCoeff =
      m_baseZoomInCoeffFactor * (1.0F + m_filterEffectsSettings->vitesse.GetRelativeSpeed());

  return {baseZoomCoeff, baseZoomCoeff};
}

inline auto ZoomVectorEffects::GetClampedZoomInCoeffs(const Point2dFlt& zoomCoeffs) const
    -> Point2dFlt
{
  return {GetClampedZoomInCoeff(zoomCoeffs.x), GetClampedZoomInCoeff(zoomCoeffs.y)};
}

inline auto ZoomVectorEffects::GetClampedZoomInCoeff(const float zoomCoeff) const -> float
{
  if (static constexpr auto MIN_ZOOM_COEFF = -4.01F; zoomCoeff < MIN_ZOOM_COEFF)
  {
    return MIN_ZOOM_COEFF;
  }
  if (zoomCoeff > m_filterEffectsSettings->maxZoomInCoeff)
  {
    return m_filterEffectsSettings->maxZoomInCoeff;
  }
  return zoomCoeff;
}

inline auto ZoomVectorEffects::GetAfterEffectsVelocity(const NormalizedCoords& coords,
                                                       const float sqDistFromZero,
                                                       const NormalizedCoords& zoomInVelocity) const
    -> NormalizedCoords
{
  return m_zoomVectorAfterEffects.GetAfterEffectsVelocity(coords, sqDistFromZero, zoomInVelocity);
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
