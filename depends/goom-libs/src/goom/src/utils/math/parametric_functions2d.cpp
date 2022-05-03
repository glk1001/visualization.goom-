#include "parametric_functions2d.h"

#include "misc.h"
#include "point2d.h"

#include <cassert>
#include <cmath>

namespace GOOM::UTILS::MATH
{

CircleFunction::CircleFunction(const Params& params) noexcept
  : m_centre{params.centrePos.ToFlt()},
    m_radius{params.radius},
    m_startAngleInRadians{ToRadians(params.angleParams.startAngleInDegrees)},
    m_endAngleInRadians{ToRadians(params.angleParams.endAngleInDegrees)}
{
}

auto CircleFunction::GetPointAtAngle(const float angle) const -> Point2dFlt
{
  return Point2dFlt{m_radius * std::cos(angle), -m_radius * std::sin(angle)} + m_centre;
}

LissajousFunction::LissajousFunction(const Params& params) noexcept
  : m_centre{params.centrePos.ToFlt()},
    m_startAngleInRadians{ToRadians(params.angleParams.startAngleInDegrees)},
    m_endAngleInRadians{ToRadians(params.angleParams.endAngleInDegrees)},
    m_lissajousParams{params.lissajousParams}
{
}

auto LissajousFunction::GetPointAtAngle(const float angle) const -> Point2dFlt
{
  return Point2dFlt{+m_lissajousParams.a * std::cos(m_lissajousParams.kX * angle),
                    -m_lissajousParams.b * std::sin(m_lissajousParams.kY * angle)} +
         m_centre;
}

HypotrochoidFunction::HypotrochoidFunction(const Params& params) noexcept
  : m_centre{params.centrePos.ToFlt()},
    m_startAngleInRadians{ToRadians(params.angleParams.startAngleInDegrees)},
    m_endAngleInRadians{ToRadians(params.angleParams.endAngleInDegrees)},
    m_hypotrochoidParams{params.hypotrochoidParams},
    m_rDiff{m_hypotrochoidParams.bigR - m_hypotrochoidParams.smallR},
    m_numCusps{GetNumCusps(m_hypotrochoidParams.bigR, m_hypotrochoidParams.smallR)}
{
  assert(m_hypotrochoidParams.bigR > 0.0F);
  assert(m_hypotrochoidParams.smallR > 0.0F);
  assert(m_hypotrochoidParams.amplitude > 0.0F);
  assert(m_startAngleInRadians <= m_endAngleInRadians);
}

auto HypotrochoidFunction::GetNumCusps(const float bigR, const float smallR) -> float
{
  const auto intBigR = static_cast<int32_t>(bigR + SMALL_FLOAT);
  const auto intSmallR = static_cast<int32_t>(smallR + SMALL_FLOAT);

  if ((0 == intBigR) || (0 == intSmallR))
  {
    return 1.0F;
  }

  return static_cast<float>(Lcm(intSmallR, intBigR) / static_cast<int64_t>(intBigR));
}

auto HypotrochoidFunction::GetPointAtAngle(const float angle) const -> Point2dFlt
{
  const float angleArg2 = (m_rDiff / m_hypotrochoidParams.smallR) * angle;

  const float x =
      +(m_rDiff * std::cos(angle)) + (m_hypotrochoidParams.height * std::cos(angleArg2));
  const float y =
      -(m_rDiff * std::sin(angle)) + (m_hypotrochoidParams.height * std::sin(angleArg2));

  return (m_hypotrochoidParams.amplitude * Point2dFlt{x, y}) + m_centre;
}

EpicycloidFunction::EpicycloidFunction(const Params& params) noexcept
  : m_centre{params.centrePos.ToFlt()},
    m_startAngleInRadians{ToRadians(params.angleParams.startAngleInDegrees)},
    m_endAngleInRadians{ToRadians(params.angleParams.endAngleInDegrees)},
    m_epicycloidParams{params.epicycloidParams},
    m_numCusps{GetNumCusps(m_epicycloidParams.k)}
{
  assert(m_epicycloidParams.k > 0.0F);
  assert(m_epicycloidParams.smallR > 0.0F);
  assert(m_epicycloidParams.amplitude > 0.0F);
  assert(m_startAngleInRadians <= m_endAngleInRadians);
}

auto EpicycloidFunction::GetNumCusps([[maybe_unused]] const float k) -> float
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

auto EpicycloidFunction::GetPointAtAngle(const float angle) const -> Point2dFlt
{
  const float angleArg2 = (m_epicycloidParams.k + 1.0F) * angle;

  const float x = +(m_epicycloidParams.smallR * (m_epicycloidParams.k + 1.0F) * std::cos(angle)) -
                  +(m_epicycloidParams.smallR * std::cos(angleArg2));
  const float y = -(m_epicycloidParams.smallR * (m_epicycloidParams.k + 1.0F) * std::sin(angle)) +
                  +(m_epicycloidParams.smallR * std::sin(angleArg2));

  return (m_epicycloidParams.amplitude * Point2dFlt{x, y}) + m_centre;
}

SineFunction::SineFunction(const Params& params) noexcept
  : m_params{params},
    m_distance{Distance(m_params.startPos.ToFlt(), m_params.endPos.ToFlt())},
    m_rotateAngle{
        std::asin((static_cast<float>(m_params.endPos.y - m_params.startPos.y)) / m_distance)}
{
}

auto SineFunction::GetPoint(const float t) const -> Point2dFlt
{
  const float y = 100.0F * std::sin(m_params.freq * TWO_PI * t);
  const float x = m_distance * t;

  Point2dFlt newPoint{x, y};
  newPoint.Rotate(m_rotateAngle);

  return ((m_params.amplitude * newPoint) + Vec2dFlt{m_params.startPos.ToFlt()});
}

OscillatingFunction::OscillatingFunction(const Params& params) : m_params{params}
{
}

auto OscillatingFunction::GetPoint(const float t) const -> Point2dFlt
{
  const Point2dFlt linearPoint = lerp(m_params.startPos.ToFlt(), m_params.endPos.ToFlt(), t);

  if (not m_params.allowOscillatingPath)
  {
    return linearPoint;
  }

  return GetOscillatingPoint(linearPoint, t);
}

inline auto OscillatingFunction::GetOscillatingPoint(const Point2dFlt& linearPoint,
                                                     const float t) const -> Point2dFlt
{
  return {
      linearPoint.x +
          (m_params.oscillatingAmplitude * std::cos(m_params.xOscillatingFreq * t * TWO_PI)),
      linearPoint.y +
          (m_params.oscillatingAmplitude * std::sin(m_params.yOscillatingFreq * t * TWO_PI)),
  };
}

} // namespace GOOM::UTILS::MATH
