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

inline auto ZoomVectorEffects::GetBaseZoomInCoeffs() const -> Point2dFlt
{
//#define NEW_WAY
#ifdef NEW_WAY
  static constexpr auto ZOOM_IN_COEFF_MULTIPLIER = 50.0F / 50.0F;
  const auto baseZoomCoeff =
      1.0F - (ZOOM_IN_COEFF_MULTIPLIER * m_filterEffectsSettings->vitesse.GetRelativeSpeed());
#else
  static constexpr auto ZOOM_IN_COEFF_MULTIPLIER = 1.0F / 50.0F;
  const auto baseZoomCoeff =
      ZOOM_IN_COEFF_MULTIPLIER * (1.0F + m_filterEffectsSettings->vitesse.GetRelativeSpeed());
#endif

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
  if (zoomCoeff > m_filterEffectsSettings->maxZoomCoeff)
  {
    return m_filterEffectsSettings->maxZoomCoeff;
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

inline auto ZoomVectorEffects::GetZoomEffectsNameValueParams() const -> GOOM::UTILS::NameValuePairs
{
  auto nameValuePairs = GOOM::UTILS::NameValuePairs{};

  GOOM::UTILS::MoveNameValuePairs(GetZoomInCoeffsNameValueParams(), nameValuePairs);
  GOOM::UTILS::MoveNameValuePairs(m_zoomVectorAfterEffects.GetZoomEffectsNameValueParams(),
                                  nameValuePairs);

  return nameValuePairs;
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
