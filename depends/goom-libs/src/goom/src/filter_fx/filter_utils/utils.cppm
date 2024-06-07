module;

#include <cmath>

export module Goom.FilterFx.FilterUtils.Utils;

import Goom.FilterFx.NormalizedCoords;
import Goom.Utils.Math.GoomRandBase;
import Goom.Utils.Math.Misc;
import Goom.Lib.AssertUtils;
import Goom.Lib.GoomTypes;
import Goom.Lib.Point2d;

export namespace GOOM::FILTER_FX::FILTER_UTILS
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
  struct Bounds
  {
    static constexpr auto DEFAULT_MIN_SIDE_LENGTH = 0.1F;
    float minSideLength                           = DEFAULT_MIN_SIDE_LENGTH;
    float probUseCentredSides                     = UTILS::MATH::HALF;

    static constexpr auto EPSILON = UTILS::MATH::SMALL_FLOAT;
    struct Rect
    {
      static constexpr auto DEFAULT_MIN_X_MIN = NormalizedCoords::MIN_COORD;
      static constexpr auto DEFAULT_MIN_X_MAX = NormalizedCoords::MAX_COORD;
      static constexpr auto DEFAULT_MIN_Y_MIN = NormalizedCoords::MIN_COORD;
      static constexpr auto DEFAULT_MIN_Y_MAX = NormalizedCoords::MAX_COORD;
      MinMaxValues<float> minMaxXMin{DEFAULT_MIN_X_MIN, DEFAULT_MIN_X_MIN + EPSILON};
      MinMaxValues<float> minMaxYMin{DEFAULT_MIN_Y_MIN, DEFAULT_MIN_Y_MIN + EPSILON};
      MinMaxValues<float> minMaxXMax{DEFAULT_MIN_X_MAX - EPSILON, DEFAULT_MIN_X_MAX};
      MinMaxValues<float> minMaxYMax{DEFAULT_MIN_Y_MAX - EPSILON, DEFAULT_MIN_Y_MAX};
    } rect;
    struct Sides
    {
      static constexpr auto DEFAULT_MIN_LENGTH = NormalizedCoords::COORD_WIDTH - EPSILON;
      static constexpr auto DEFAULT_MAX_LENGTH = NormalizedCoords::COORD_WIDTH - EPSILON;
      MinMaxValues<float> minMaxWidth{DEFAULT_MIN_LENGTH, DEFAULT_MAX_LENGTH + EPSILON};
      MinMaxValues<float> minMaxHeight{DEFAULT_MIN_LENGTH, DEFAULT_MAX_LENGTH + EPSILON};
    } sides;
  };

  static constexpr auto DEFAULT_PROB_NO_VIEWPORT     = 0.1F;
  static constexpr auto DEFAULT_PROB_SQUARE_VIEWPORT = 0.8F;

  RandomViewport(const UTILS::MATH::IGoomRand& goomRand, const Bounds& bounds) noexcept;

  [[nodiscard]] auto GetProbNoViewport() const noexcept -> float;
  auto SetProbNoViewport(float probNoViewport) noexcept -> void;

  [[nodiscard]] auto GetProbSquareViewport() const noexcept -> float;
  auto SetProbSquareViewport(float probSquareViewport) noexcept -> void;

  [[nodiscard]] auto GetMinSideLength() const noexcept -> float;
  auto SetMinSideLength(float minSideLength) noexcept -> void;

  [[nodiscard]] auto GetMinMaxXMin() const noexcept -> MinMaxValues<float>;
  [[nodiscard]] auto GetMinMaxXMax() const noexcept -> MinMaxValues<float>;
  [[nodiscard]] auto GetMinMaxYMin() const noexcept -> MinMaxValues<float>;
  [[nodiscard]] auto GetMinMaxYMax() const noexcept -> MinMaxValues<float>;
  auto SetBoundsRect(const Bounds::Rect& rect) noexcept -> void;

  [[nodiscard]] auto GetMinMaxWidth() const noexcept -> MinMaxValues<float>;
  [[nodiscard]] auto GetMinMaxHeight() const noexcept -> MinMaxValues<float>;
  auto SetBoundsSides(const Bounds::Sides& sides) noexcept -> void;

  [[nodiscard]] auto GetRandomViewport() const noexcept -> Viewport;

private:
  const UTILS::MATH::IGoomRand* m_goomRand;
  float m_probNoViewport     = DEFAULT_PROB_NO_VIEWPORT;
  float m_probSquareViewport = DEFAULT_PROB_SQUARE_VIEWPORT;

  Bounds m_bounds;
  [[nodiscard]] auto GetRandomUncentredViewport() const noexcept -> Viewport;
  [[nodiscard]] auto GetRandomCentredViewport() const noexcept -> Viewport;
};

} // namespace GOOM::FILTER_FX::FILTER_UTILS

namespace GOOM::FILTER_FX::FILTER_UTILS
{

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

inline auto RandomViewport::GetMinSideLength() const noexcept -> float
{
  return m_bounds.minSideLength;
}

inline auto RandomViewport::SetMinSideLength(float minSideLength) noexcept -> void
{
  m_bounds.minSideLength = minSideLength;
}

inline auto RandomViewport::GetMinMaxXMin() const noexcept -> MinMaxValues<float>
{
  return m_bounds.rect.minMaxXMin;
}

inline auto RandomViewport::GetMinMaxXMax() const noexcept -> MinMaxValues<float>
{
  return m_bounds.rect.minMaxXMax;
}

inline auto RandomViewport::GetMinMaxYMin() const noexcept -> MinMaxValues<float>
{
  return m_bounds.rect.minMaxYMin;
}

inline auto RandomViewport::GetMinMaxYMax() const noexcept -> MinMaxValues<float>
{
  return m_bounds.rect.minMaxYMax;
}

inline auto RandomViewport::SetBoundsRect(const Bounds::Rect& rect) noexcept -> void
{
  Expects(rect.minMaxXMin.minValue < rect.minMaxXMin.maxValue);
  Expects(rect.minMaxXMax.minValue < rect.minMaxXMax.maxValue);
  Expects(rect.minMaxYMin.minValue < rect.minMaxYMin.maxValue);
  Expects(rect.minMaxYMax.minValue < rect.minMaxYMax.maxValue);
  m_bounds.rect = rect;
}

inline auto RandomViewport::GetMinMaxWidth() const noexcept -> MinMaxValues<float>
{
  return m_bounds.sides.minMaxWidth;
}

inline auto RandomViewport::GetMinMaxHeight() const noexcept -> MinMaxValues<float>
{
  return m_bounds.sides.minMaxHeight;
}

inline auto RandomViewport::SetBoundsSides(const Bounds::Sides& sides) noexcept -> void
{
  Expects(sides.minMaxWidth.minValue > 0.0F);
  Expects(sides.minMaxHeight.minValue > 0.0F);
  Expects(sides.minMaxWidth.minValue < sides.minMaxWidth.maxValue);
  Expects(sides.minMaxHeight.minValue < sides.minMaxHeight.maxValue);
  m_bounds.sides = sides;
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
      std::lerp(coords.GetX(), 1.0F, lerpToOneTs.xLerpT),
      std::lerp(coords.GetY(), 1.0F, lerpToOneTs.yLerpT),
  };
}

inline auto GetDiscontinuousXyZoomFactor(const NormalizedCoords& coords,
                                         const LerpToOneTs& lerpToOneTs) noexcept -> XyZoomFactor
{
  const auto xAbsZoomFactor = std::lerp(std::abs(coords.GetX()), 1.0F, lerpToOneTs.xLerpT);
  const auto yAbsZoomFactor = std::lerp(std::abs(coords.GetY()), 1.0F, lerpToOneTs.yLerpT);

  return {
      coords.GetX() < 0.0F ? -xAbsZoomFactor : +xAbsZoomFactor,
      coords.GetY() < 0.0F ? -yAbsZoomFactor : +yAbsZoomFactor,
  };
}

} // namespace GOOM::FILTER_FX::FILTER_UTILS
