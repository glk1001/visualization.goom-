#pragma once

#include "filter_fx/after_effects/zoom_vector_after_effects.h"
#include "filter_fx/filter_settings.h"
#include "filter_fx/normalized_coords.h"
#include "filter_fx/zoom_in_coefficients_effect.h"
#include "point2d.h"
#include "utils/math/goom_rand_base.h"
#include "utils/name_value_pairs.h"

#include <functional>

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

class ZoomVectorEffects
{
public:
  using GetAfterEffectsFunc = std::function<auto(const UTILS::MATH::IGoomRand& goomRand,
                                                 const std::string& resourcesDirectory)
                                                ->AFTER_EFFECTS::AfterEffects>;

  ZoomVectorEffects(uint32_t screenWidth,
                    const std::string& resourcesDirectory,
                    const UTILS::MATH::IGoomRand& goomRand,
                    const GetAfterEffectsFunc& getAfterEffects) noexcept;

  static constexpr auto IsValidMultiplierRange(
      const UTILS::MATH::IGoomRand::NumberRange<float>& range) noexcept -> bool;

  [[nodiscard]] static auto GetStandardAfterEffects(const UTILS::MATH::IGoomRand& goomRand,
                                                    const std::string& resourcesDirectory) noexcept
      -> AFTER_EFFECTS::AfterEffects;

  auto SetFilterSettings(const FilterEffectsSettings& filterEffectsSettings) noexcept -> void;

  [[nodiscard]] auto GetZoomInCoefficients(const NormalizedCoords& coords,
                                           float sqDistFromZero) const noexcept -> Point2dFlt;

  [[nodiscard]] auto GetAfterEffectsVelocity(const NormalizedCoords& coords,
                                             float sqDistFromZero,
                                             const NormalizedCoords& zoomInVelocity) const noexcept
      -> NormalizedCoords;
  [[nodiscard]] auto GetAfterEffectsVelocityMultiplier() const noexcept -> float;

  [[nodiscard]] auto GetZoomEffectsNameValueParams() const noexcept -> UTILS::NameValuePairs;

protected:
  static constexpr auto RAW_BASE_ZOOM_IN_COEFF_FACTOR = 0.02F;

private:
  const FilterEffectsSettings* m_filterEffectsSettings{};
  AFTER_EFFECTS::ZoomVectorAfterEffects m_zoomVectorAfterEffects;

  static constexpr auto MIN_ALLOWED_BASE_ZOOM_IN_COEFF = 0.0F;
  static constexpr auto MAX_ALLOWED_BASE_ZOOM_IN_COEFF = 0.5F;
  float m_baseZoomInCoeffFactor                        = RAW_BASE_ZOOM_IN_COEFF_FACTOR;
  auto SetBaseZoomInCoeffFactor(float multiplier) noexcept -> void;
  auto SetBaseZoomInCoeffs() noexcept -> void;
  [[nodiscard]] static constexpr auto GetBaseZoomInCoeff(float baseZoomInCoeffFactor,
                                                         float relativeSpeed) noexcept -> float;

  [[nodiscard]] auto GetClampedZoomInCoeffs(const Point2dFlt& zoomCoeffs) const noexcept
      -> Point2dFlt;
  [[nodiscard]] auto GetClampedZoomInCoeff(float zoomInCoeff) const noexcept -> float;

  [[nodiscard]] auto GetZoomInCoeffsNameValueParams() const noexcept -> UTILS::NameValuePairs;
};

constexpr auto ZoomVectorEffects::IsValidMultiplierRange(
    const UTILS::MATH::IGoomRand::IGoomRand::NumberRange<float>& range) noexcept -> bool
{
  if (MIN_ALLOWED_BASE_ZOOM_IN_COEFF >
      GetBaseZoomInCoeff(range.min * RAW_BASE_ZOOM_IN_COEFF_FACTOR, -1.0F))
  {
    return false;
  }
  if (MIN_ALLOWED_BASE_ZOOM_IN_COEFF >
      GetBaseZoomInCoeff(range.max * RAW_BASE_ZOOM_IN_COEFF_FACTOR, -1.0F))
  {
    return false;
  }
  if (MAX_ALLOWED_BASE_ZOOM_IN_COEFF <
      GetBaseZoomInCoeff(range.min * RAW_BASE_ZOOM_IN_COEFF_FACTOR, +1.0F))
  {
    return false;
  }
  if (MAX_ALLOWED_BASE_ZOOM_IN_COEFF <
      GetBaseZoomInCoeff(range.max * RAW_BASE_ZOOM_IN_COEFF_FACTOR, +1.0F))
  {
    return false;
  }

  return true;
}

inline auto ZoomVectorEffects::SetBaseZoomInCoeffFactor(const float multiplier) noexcept -> void
{
  Expects(multiplier > 0.0F);

  m_baseZoomInCoeffFactor = multiplier * RAW_BASE_ZOOM_IN_COEFF_FACTOR;

  Ensures(GetBaseZoomInCoeff(m_baseZoomInCoeffFactor,
                             m_filterEffectsSettings->vitesse.GetRelativeSpeed()) <=
          MAX_ALLOWED_BASE_ZOOM_IN_COEFF);

  SetBaseZoomInCoeffs();
}

inline auto ZoomVectorEffects::SetBaseZoomInCoeffs() noexcept -> void
{
  // TODO(glk) Does GetRelativeSpeed change in between this setter use?
  const auto baseZoomInCoeff = GetBaseZoomInCoeff(
      m_baseZoomInCoeffFactor, m_filterEffectsSettings->vitesse.GetRelativeSpeed());

  Ensures(MIN_ALLOWED_BASE_ZOOM_IN_COEFF <= baseZoomInCoeff);
  Ensures(baseZoomInCoeff <= MAX_ALLOWED_BASE_ZOOM_IN_COEFF);

  m_filterEffectsSettings->zoomInCoefficientsEffect->SetBaseZoomInCoeffs(
      {baseZoomInCoeff, baseZoomInCoeff});
}

constexpr auto ZoomVectorEffects::GetBaseZoomInCoeff(const float baseZoomInCoeffFactor,
                                                     const float relativeSpeed) noexcept -> float
{
  return baseZoomInCoeffFactor * (1.0F + relativeSpeed);
}

inline auto ZoomVectorEffects::GetZoomInCoefficients(const NormalizedCoords& coords,
                                                     const float sqDistFromZero) const noexcept
    -> Point2dFlt
{
  // Amulet 2
  // vx = X * tan(dist);
  // vy = Y * tan(dist);
  return GetClampedZoomInCoeffs(
      m_filterEffectsSettings->zoomInCoefficientsEffect->GetZoomInCoefficients(coords,
                                                                               sqDistFromZero));
}

inline auto ZoomVectorEffects::GetClampedZoomInCoeffs(const Point2dFlt& zoomCoeffs) const noexcept
    -> Point2dFlt
{
  return {GetClampedZoomInCoeff(zoomCoeffs.x), GetClampedZoomInCoeff(zoomCoeffs.y)};
}

inline auto ZoomVectorEffects::GetClampedZoomInCoeff(const float zoomInCoeff) const noexcept
    -> float
{
  if (static constexpr auto MIN_ZOOM_IN_COEFF = -4.01F; zoomInCoeff < MIN_ZOOM_IN_COEFF)
  {
    return MIN_ZOOM_IN_COEFF;
  }
  if (zoomInCoeff > m_filterEffectsSettings->maxZoomInCoeff)
  {
    return m_filterEffectsSettings->maxZoomInCoeff;
  }
  return zoomInCoeff;
}

inline auto ZoomVectorEffects::GetAfterEffectsVelocity(
    const NormalizedCoords& coords,
    const float sqDistFromZero,
    const NormalizedCoords& zoomInVelocity) const noexcept -> NormalizedCoords
{
  return m_zoomVectorAfterEffects.GetAfterEffectsVelocity(coords, sqDistFromZero, zoomInVelocity);
}

inline auto ZoomVectorEffects::GetAfterEffectsVelocityMultiplier() const noexcept -> float
{
  return m_filterEffectsSettings->afterEffectsVelocityMultiplier;
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
