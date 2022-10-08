#pragma once

#include "filter_fx/normalized_coords.h"
#include "point2d.h"
#include "utils/name_value_pairs.h"

namespace GOOM::FILTER_FX
{

class IZoomInCoefficientsEffect
{
public:
  IZoomInCoefficientsEffect() noexcept                                           = default;
  IZoomInCoefficientsEffect(const IZoomInCoefficientsEffect&) noexcept           = default;
  IZoomInCoefficientsEffect(IZoomInCoefficientsEffect&&) noexcept                = default;
  virtual ~IZoomInCoefficientsEffect() noexcept                                  = default;
  auto operator=(const IZoomInCoefficientsEffect&) -> IZoomInCoefficientsEffect& = default;
  auto operator=(IZoomInCoefficientsEffect&&) -> IZoomInCoefficientsEffect&      = default;

  virtual auto SetRandomParams() -> void = 0;

  auto SetBaseZoomInCoeffs(const Point2dFlt& baseZoomInCoeffs) noexcept -> void;

  [[nodiscard]] virtual auto GetZoomInCoefficients(const NormalizedCoords& coords,
                                                   float sqDistFromZero) const -> Point2dFlt = 0;

  [[nodiscard]] virtual auto GetZoomInCoefficientsEffectNameValueParams() const
      -> GOOM::UTILS::NameValuePairs = 0;

protected:
  [[nodiscard]] auto GetBaseZoomInCoeffs() const noexcept -> const Point2dFlt&;

private:
  Point2dFlt m_baseZoomInCoeffs{};
};

inline auto IZoomInCoefficientsEffect::GetBaseZoomInCoeffs() const noexcept -> const Point2dFlt&
{
  return m_baseZoomInCoeffs;
}

inline auto IZoomInCoefficientsEffect::SetBaseZoomInCoeffs(
    const Point2dFlt& baseZoomInCoeffs) noexcept -> void
{
  m_baseZoomInCoeffs = baseZoomInCoeffs;
}

} // namespace GOOM::FILTER_FX
