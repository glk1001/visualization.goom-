#pragma once

#include "goom/goom_config.h"
#include "goom/goom_types.h"
#include "goom/point2d.h"
#include "line_clipper.h"
#include "utils/math/goom_rand_base.h"
#include "utils/math/misc.h"

#include <algorithm>
#include <cstdint>

namespace GOOM::UTILS::GRAPHICS
{

[[nodiscard]] constexpr auto IsZero(const Point2dInt& point) noexcept -> bool;

[[nodiscard]] constexpr auto GetMinSideLength(const Rectangle2dInt& rectangle2D) noexcept
    -> uint32_t;

[[nodiscard]] constexpr auto GetCentrePoint(const Rectangle2dInt& rectangle2D) noexcept
    -> Point2dInt;

[[nodiscard]] auto GetPointClippedToRectangle(
    const Point2dInt& point,
    const Rectangle2dInt& clipRectangle,
    const Point2dInt& connectingPointInsideClipRectangle) noexcept -> Point2dInt;

[[nodiscard]] auto GetRandomPoint(const UTILS::MATH::IGoomRand& goomRand,
                                  const Rectangle2dInt& rectangle2D) noexcept -> Point2dInt;

inline constexpr auto DEFAULT_CLOSE_TO_WEIGHT_POINT_T = 0.3F; // not very close
[[nodiscard]] auto GetRandomPoint(
    const UTILS::MATH::IGoomRand& goomRand,
    const Rectangle2dInt& rectangle2D,
    const Point2dInt& weightPoint,
    float closeToWeightPointT = DEFAULT_CLOSE_TO_WEIGHT_POINT_T) noexcept -> Point2dInt;

constexpr auto IsZero(const Point2dInt& point) noexcept -> bool
{
  return (0 == point.x) and (0 == point.y);
}

constexpr auto GetMinSideLength(const Rectangle2dInt& rectangle2D) noexcept -> uint32_t
{
  Expects(rectangle2D.topLeft.x <= rectangle2D.bottomRight.x);
  Expects(rectangle2D.topLeft.y <= rectangle2D.bottomRight.y);

  return static_cast<uint32_t>(std::min(rectangle2D.bottomRight.x - rectangle2D.topLeft.x,
                                        rectangle2D.bottomRight.y - rectangle2D.topLeft.y));
}

constexpr auto GetCentrePoint(const Rectangle2dInt& rectangle2D) noexcept -> Point2dInt
{
  Expects(rectangle2D.topLeft.x <= rectangle2D.bottomRight.x);
  Expects(rectangle2D.topLeft.y <= rectangle2D.bottomRight.y);

  return {UTILS::MATH::I_HALF * (rectangle2D.topLeft.x + rectangle2D.bottomRight.x),
          UTILS::MATH::I_HALF * (rectangle2D.topLeft.y + rectangle2D.bottomRight.y)};
}

inline auto GetPointClippedToRectangle(
    const Point2dInt& point,
    const Rectangle2dInt& clipRectangle,
    const Point2dInt& connectingPointInsideClipRectangle) noexcept -> Point2dInt
{
  const auto lineClipper = LineClipper{clipRectangle};
  const auto clippedLine = lineClipper.GetClippedLine(
      LineFlt{ToPoint2dFlt(connectingPointInsideClipRectangle), ToPoint2dFlt(point)});
  Expects(clippedLine.clipResult != LineClipper::ClipResult::REJECTED);

  return ToPoint2dInt(clippedLine.line.point2);
}

inline auto GetRandomPoint(const UTILS::MATH::IGoomRand& goomRand,
                           const Rectangle2dInt& rectangle2D) noexcept -> Point2dInt
{
  Expects(rectangle2D.topLeft.x <= rectangle2D.bottomRight.x);
  Expects(rectangle2D.topLeft.y <= rectangle2D.bottomRight.y);

  return {
      goomRand.GetRandInRange(rectangle2D.topLeft.x, rectangle2D.bottomRight.x + 1),
      goomRand.GetRandInRange(rectangle2D.topLeft.y, rectangle2D.bottomRight.y + 1),
  };
}

inline auto GetRandomPoint(const UTILS::MATH::IGoomRand& goomRand,
                           const Rectangle2dInt& rectangle2D,
                           const Point2dInt& weightPoint,
                           const float closeToWeightPointT) noexcept -> Point2dInt
{
  const auto unweightedPoint = GetRandomPoint(goomRand, rectangle2D);

  return lerp(unweightedPoint, weightPoint, closeToWeightPointT);
}

} // namespace GOOM::UTILS::GRAPHICS
