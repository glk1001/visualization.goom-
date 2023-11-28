#pragma once

#include "filter_fx/normalized_coords.h"
#include "goom/math20.h"
#include "goom/point2d.h"

#include <cmath>

namespace GOOM::FILTER_FX::FILTER_UTILS
{

struct LerpToOneTs
{
  float xLerpT;
  float yLerpT;
};
struct XyZoomFactor
{
  float xFactor;
  float yFactor;
};

[[nodiscard]] auto GetVelocityByZoomLerpedToOne(const NormalizedCoords& coords,
                                                const LerpToOneTs& lerpToOneTs,
                                                const Vec2dFlt& velocity) noexcept -> Vec2dFlt;
[[nodiscard]] auto GetVelocityByZoomLerpedToNegOne(const NormalizedCoords& coords,
                                                   const LerpToOneTs& lerpToOneTs,
                                                   const Vec2dFlt& velocity) noexcept -> Vec2dFlt;
[[nodiscard]] auto GetAppliedZoomFactor(const XyZoomFactor& zoomFactor,
                                        const Vec2dFlt& velocity) noexcept -> Vec2dFlt;
[[nodiscard]] auto GetXyZoomFactor(const NormalizedCoords& coords,
                                   const LerpToOneTs& lerpToOneTs) noexcept -> XyZoomFactor;
[[nodiscard]] auto GetDiscontinuousXyZoomFactor(const NormalizedCoords& coords,
                                                const LerpToOneTs& lerpToOneTs) noexcept
    -> XyZoomFactor;

inline auto GetVelocityByZoomLerpedToOne(const NormalizedCoords& coords,
                                         const LerpToOneTs& lerpToOneTs,
                                         const Vec2dFlt& velocity) noexcept -> Vec2dFlt
{
  return GetAppliedZoomFactor(GetXyZoomFactor(coords, lerpToOneTs), velocity);
}

inline auto GetVelocityByZoomLerpedToNegOne(const NormalizedCoords& coords,
                                            const LerpToOneTs& lerpToOneTs,
                                            const Vec2dFlt& velocity) noexcept -> Vec2dFlt
{
  return GetAppliedZoomFactor(GetDiscontinuousXyZoomFactor(coords, lerpToOneTs), velocity);
}

inline auto GetAppliedZoomFactor(const XyZoomFactor& zoomFactor, const Vec2dFlt& velocity) noexcept
    -> Vec2dFlt
{
  return {zoomFactor.xFactor * velocity.x, zoomFactor.yFactor * velocity.y};
}

inline auto GetXyZoomFactor(const NormalizedCoords& coords, const LerpToOneTs& lerpToOneTs) noexcept
    -> XyZoomFactor
{
  return {
      STD20::lerp(coords.GetX(), 1.0F, lerpToOneTs.xLerpT),
      STD20::lerp(coords.GetY(), 1.0F, lerpToOneTs.yLerpT),
  };
}

inline auto GetDiscontinuousXyZoomFactor(const NormalizedCoords& coords,
                                         const LerpToOneTs& lerpToOneTs) noexcept -> XyZoomFactor
{
  const auto xAbsZoomFactor = STD20::lerp(std::abs(coords.GetX()), 1.0F, lerpToOneTs.xLerpT);
  const auto yAbsZoomFactor = STD20::lerp(std::abs(coords.GetY()), 1.0F, lerpToOneTs.yLerpT);

  return {
      coords.GetX() < 0.0F ? -xAbsZoomFactor : +xAbsZoomFactor,
      coords.GetY() < 0.0F ? -yAbsZoomFactor : +yAbsZoomFactor,
  };
}

} // namespace GOOM::FILTER_FX::FILTER_UTILS
