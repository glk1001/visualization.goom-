#include "paths.h"

#include "misc.h"
#include "point2d.h"

#include <cassert>
#include <cmath>

namespace GOOM::UTILS::MATH
{

using MATH::Transform2d;

TransformedPath::TransformedPath(std::shared_ptr<IPath> path, const Transform2d& transform)
  : IPath{path->m_positionT}, m_path{std::move(path)}, m_transform{transform}
{
}

LerpedPath::LerpedPath(TValue& positionT,
                       std::shared_ptr<IPath> path1,
                       std::shared_ptr<IPath> path2,
                       TValue& lerpT)
  : IPath{positionT}, m_path1{std::move(path1)}, m_path2{std::move(path2)}, m_lerpT{lerpT}
{
  assert(&positionT == &path1.m_positionT);
  assert(&positionT == &path2.m_positionT);
}

CirclePath::CirclePath(const Point2dInt& centrePos,
                       TValue& positionT,
                       const float radius,
                       const AngleParams& angleParams) noexcept
  : IPath{positionT},
    m_centre{centrePos.ToFlt()},
    m_radius{radius},
    m_startAngleInRadians{ToRadians(angleParams.startAngleInDegrees)},
    m_endAngleInRadians{ToRadians(angleParams.endAngleInDegrees)}
{
}

auto CirclePath::GetPoint(const float angle) const -> Point2dFlt
{
  return Point2dFlt{m_radius * std::cos(angle), -m_radius * std::sin(angle)} + m_centre;
}

LissajousPath::LissajousPath(const Point2dInt& centrePos,
                             TValue& positionT,
                             const Params& params,
                             const AngleParams& angleParams) noexcept
  : IPath{positionT},
    m_centre{centrePos.ToFlt()},
    m_params{params},
    m_startAngleInRadians{ToRadians(angleParams.startAngleInDegrees)},
    m_endAngleInRadians{ToRadians(angleParams.endAngleInDegrees)}
{
}

auto LissajousPath::GetPoint(const float angle) const -> Point2dFlt
{
  return Point2dFlt{m_params.a * std::cos(m_params.kX * angle),
                    -m_params.b * std::sin(m_params.kY * angle)} +
         m_centre;
}

Hypotrochoid::Hypotrochoid(const Point2dInt& centrePos,
                           TValue& positionT,
                           const Params& params,
                           const AngleParams& angleParams) noexcept
  : IPath{positionT},
    m_centre{centrePos.ToFlt()},
    m_params{params},
    m_startAngleInRadians{ToRadians(angleParams.startAngleInDegrees)},
    m_endAngleInRadians{ToRadians(angleParams.endAngleInDegrees)},
    m_rDiff{m_params.bigR - m_params.smallR},
    m_numCusps{GetNumCusps(m_params.bigR, m_params.smallR)}
{
  assert(m_params.bigR > 0.0F);
  assert(m_params.smallR > 0.0F);
  assert(m_params.amplitude > 0.0F);
  assert(m_startAngleInRadians <= m_endAngleInRadians);
}

auto Hypotrochoid::GetNumCusps(const float bigR, const float smallR) -> float
{
  const auto intBigR = static_cast<int32_t>(bigR + SMALL_FLOAT);
  const auto intSmallR = static_cast<int32_t>(smallR + SMALL_FLOAT);

  if ((0 == intBigR) || (0 == intSmallR))
  {
    return 1.0F;
  }

  return static_cast<float>(Lcm(intSmallR, intBigR) / static_cast<int64_t>(intBigR));
}

auto Hypotrochoid::GetPoint(const float angle) const -> Point2dFlt
{
  const float angleArg2 = (m_rDiff / m_params.smallR) * angle;

  const float x = +(m_rDiff * std::cos(angle)) + (m_params.height * std::cos(angleArg2));
  const float y = -(m_rDiff * std::sin(angle)) + (m_params.height * std::sin(angleArg2));

  return (m_params.amplitude * Point2dFlt{x, y}) + m_centre;
}

Epicycloid::Epicycloid(const Point2dInt& centrePos,
                       TValue& positionT,
                       const Params& params,
                       const AngleParams& angleParams) noexcept
  : IPath{positionT},
    m_centre{centrePos.ToFlt()},
    m_params{params},
    m_startAngleInRadians{ToRadians(angleParams.startAngleInDegrees)},
    m_endAngleInRadians{ToRadians(angleParams.endAngleInDegrees)},
    m_numCusps{GetNumCusps(m_params.k)}
{
  assert(m_params.k > 0.0F);
  assert(m_params.smallR > 0.0F);
  assert(m_params.amplitude > 0.0F);
  assert(m_startAngleInRadians <= m_endAngleInRadians);
}

auto Epicycloid::GetNumCusps([[maybe_unused]] const float k) -> float
{
  // From 'https://en.wikipedia.org/wiki/Epicycloid'
  if (const RationalNumber frac = FloatToIrreducibleFraction(k); frac.isRational)
  {
    return static_cast<float>(frac.numerator);
  }

  // k is irrational. Curve never closes, so return 'large' number.
  static constexpr float LARGE_NUM_CUSPS = 20.0F;
  return LARGE_NUM_CUSPS;
}

auto Epicycloid::GetPoint(const float angle) const -> Point2dFlt
{
  const float angleArg2 = (m_params.k + 1.0F) * angle;

  const float x = +(m_params.smallR * (m_params.k + 1.0F) * std::cos(angle)) -
                  (m_params.smallR * std::cos(angleArg2));
  const float y = -(m_params.smallR * (m_params.k + 1.0F) * std::sin(angle)) +
                  (m_params.smallR * std::sin(angleArg2));

  return (m_params.amplitude * Point2dFlt{x, y}) + m_centre;
}

SinePath::SinePath(const Point2dInt& startPos,
                   const Point2dInt& endPos,
                   TValue& positionT,
                   const Params& params) noexcept
  : IPathWithStartAndEnd{startPos, endPos, positionT},
    m_params{params},
    m_distance{Distance(startPos.ToFlt(), endPos.ToFlt())},
    m_rotateAngle{std::asin((static_cast<float>(endPos.y - startPos.y)) / m_distance)}
{
}

auto SinePath::GetNextPoint() const -> Point2dInt
{
  const float y = 100.0F * std::sin(m_params.freq * TWO_PI * GetCurrentT());
  const float x = m_distance * GetCurrentT();

  Point2dFlt newPoint{x, y};
  newPoint.Rotate(m_rotateAngle);

  return ((m_params.amplitude * newPoint) + Vec2dFlt{GetStartPos().ToFlt()}).ToInt();
}

OscillatingPath::OscillatingPath(const Point2dInt& startPos,
                                 const Point2dInt& endPos,
                                 TValue& t,
                                 const Params& params,
                                 const bool allowOscillatingPath)
  : IPathWithStartAndEnd{startPos, endPos, t},
    m_params{params},
    m_allowOscillatingPath{allowOscillatingPath}
{
}

auto OscillatingPath::GetNextPoint() const -> Point2dInt
{
  const Point2dFlt linearPoint = lerp(GetStartPos().ToFlt(), GetEndPos().ToFlt(), GetCurrentT());

  if (!m_allowOscillatingPath)
  {
    return {static_cast<int32_t>(std::round(linearPoint.x)),
            static_cast<int32_t>(std::round(linearPoint.y))};
  }

  const Point2dFlt finalPoint = GetOscillatingPointAtNextT(linearPoint);
  return {static_cast<int32_t>(std::round(finalPoint.x)),
          static_cast<int32_t>(std::round(finalPoint.y))};
}

inline auto OscillatingPath::GetOscillatingPointAtNextT(const Point2dFlt& point) const -> Point2dFlt
{
  return {
      point.x + (m_params.oscillatingAmplitude *
                 std::cos(m_params.xOscillatingFreq * GetCurrentT() * TWO_PI)),
      point.y + (m_params.oscillatingAmplitude *
                 std::sin(m_params.yOscillatingFreq * GetCurrentT() * TWO_PI)),
  };
}

} // namespace GOOM::UTILS::MATH
