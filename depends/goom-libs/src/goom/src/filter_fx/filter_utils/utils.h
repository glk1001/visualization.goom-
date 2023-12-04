#pragma once

#include "filter_fx/normalized_coords.h"
#include "goom/goom_config.h"
#include "goom/goom_types.h"
#include "goom/math20.h"
#include "goom/point2d.h"
#include "utils/math/goom_rand_base.h"

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

class RandomViewport
{
public:
  static constexpr auto DEFAULT_PROB_NO_VIEWPORT     = 0.1F;
  static constexpr auto DEFAULT_PROB_SQUARE_VIEWPORT = 0.8F;

  explicit RandomViewport(const UTILS::MATH::IGoomRand& goomRand) noexcept;

  [[nodiscard]] auto GetProbNoViewport() const noexcept -> float;
  auto SetProbNoViewport(float probNoViewport) noexcept -> void;

  [[nodiscard]] auto GetProbSquareViewport() const noexcept -> float;
  auto SetProbSquareViewport(float probSquareViewport) noexcept -> void;

  static constexpr auto MARGIN      = 0.05F;
  static constexpr auto TWO_MARGINS = 2.0F * MARGIN;
  [[nodiscard]] auto GetMinMaxXWidths() const noexcept -> MinMaxValues<float>;
  auto SetMinMaxXWidths(const MinMaxValues<float>& minMaxXWidths) noexcept -> void;
  [[nodiscard]] auto GetMinMaxYHeights() const noexcept -> MinMaxValues<float>;
  auto SetMinMaxYHeights(const MinMaxValues<float>& minMaxYHeights) noexcept -> void;

  [[nodiscard]] auto GetRandomViewport() const noexcept -> Viewport;

private:
  const UTILS::MATH::IGoomRand* m_goomRand;
  float m_probNoViewport     = DEFAULT_PROB_NO_VIEWPORT;
  float m_probSquareViewport = DEFAULT_PROB_SQUARE_VIEWPORT;

  MinMaxValues<float> m_minMaxXWidths;
  MinMaxValues<float> m_minMaxYHeights;
  static constexpr auto MIN_X_WIDTH  = 1.5F;
  static constexpr auto MAX_X_WIDTH  = NormalizedCoords::COORD_WIDTH - TWO_MARGINS;
  static constexpr auto MIN_Y_HEIGHT = 1.5F;
  static constexpr auto MAX_Y_HEIGHT = NormalizedCoords::COORD_WIDTH - TWO_MARGINS;
  static_assert(MARGIN <= MIN_X_WIDTH);
  static_assert(MARGIN <= MIN_Y_HEIGHT);
};

inline auto RandomViewport::GetProbNoViewport() const noexcept -> float
{
  return m_probNoViewport;
}

inline auto RandomViewport::SetProbNoViewport(const float probNoViewport) noexcept -> void
{
  m_probNoViewport = probNoViewport;
}

inline auto RandomViewport::GetProbSquareViewport() const noexcept -> float
{
  return m_probSquareViewport;
}

inline auto RandomViewport::SetProbSquareViewport(const float probSquareViewport) noexcept -> void
{
  m_probSquareViewport = probSquareViewport;
}

inline auto RandomViewport::GetMinMaxXWidths() const noexcept -> MinMaxValues<float>
{
  return m_minMaxXWidths;
}

inline auto RandomViewport::SetMinMaxXWidths(const MinMaxValues<float>& minMaxXWidths) noexcept
    -> void
{
  Expects(minMaxXWidths.minValue < minMaxXWidths.maxValue);
  m_minMaxXWidths = minMaxXWidths;
}

inline auto RandomViewport::GetMinMaxYHeights() const noexcept -> MinMaxValues<float>
{
  return m_minMaxYHeights;
}

inline auto RandomViewport::SetMinMaxYHeights(const MinMaxValues<float>& minMaxYHeights) noexcept
    -> void
{
  Expects(minMaxYHeights.minValue < minMaxYHeights.maxValue);
  m_minMaxYHeights = minMaxYHeights;
}

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
