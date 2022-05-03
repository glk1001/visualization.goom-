#pragma once

#include "misc.h"
#include "point2d.h"

namespace GOOM::UTILS::MATH
{

class IParametricFunction2d
{
public:
  IParametricFunction2d() noexcept = default;
  IParametricFunction2d(const IParametricFunction2d&) noexcept = delete;
  IParametricFunction2d(IParametricFunction2d&&) noexcept = delete;
  virtual ~IParametricFunction2d() noexcept = default;
  auto operator=(const IParametricFunction2d&) -> IParametricFunction2d& = delete;
  auto operator=(IParametricFunction2d&&) -> IParametricFunction2d& = delete;

  [[nodiscard]] virtual auto GetPoint(float t) const -> Point2dFlt = 0;
};

class LineFunction : public IParametricFunction2d
{
public:
  struct Params
  {
    Point2dInt startPos;
    Point2dInt endPos;
  };

  explicit LineFunction(const Params& params) noexcept;

  [[nodiscard]] auto GetPoint(float t) const -> Point2dFlt override;

private:
  const Point2dFlt m_startPos;
  const Point2dFlt m_endPos;
};

struct AngleParams
{
  float startAngleInDegrees = 0.0F;
  float endAngleInDegrees = DEGREES_360;
};

class CircleFunction : public IParametricFunction2d
{
public:
  struct Params
  {
    Point2dInt centrePos;
    float radius;
    AngleParams angleParams;
  };

  explicit CircleFunction(const Params& params) noexcept;

  [[nodiscard]] auto GetPoint(float t) const -> Point2dFlt override;

private:
  const Vec2dFlt m_centre;
  const float m_radius;
  const float m_startAngleInRadians;
  const float m_endAngleInRadians;
  [[nodiscard]] auto GetPointAtAngle(float angle) const -> Point2dFlt;
};

class LissajousFunction : public IParametricFunction2d
{
public:
  struct LissajousParams
  {
    float a;
    float b;
    float kX;
    float kY;
  };
  struct Params
  {
    Point2dInt centrePos;
    AngleParams angleParams;
    LissajousParams lissajousParams;
  };

  explicit LissajousFunction(const Params& params) noexcept;

  [[nodiscard]] auto GetPoint(float t) const -> Point2dFlt override;

private:
  const Vec2dFlt m_centre;
  const float m_startAngleInRadians;
  const float m_endAngleInRadians;
  const LissajousParams m_lissajousParams;
  [[nodiscard]] auto GetPointAtAngle(float angle) const -> Point2dFlt;
};

class HypotrochoidFunction : public IParametricFunction2d
{
public:
  struct HypotrochoidParams
  {
    float bigR;
    float smallR;
    float height;
    float amplitude;
  };
  struct Params
  {
    Point2dInt centrePos;
    AngleParams angleParams;
    HypotrochoidParams hypotrochoidParams;
  };

  explicit HypotrochoidFunction(const Params& params) noexcept;

  [[nodiscard]] auto GetPoint(float t) const -> Point2dFlt override;

private:
  const Vec2dFlt m_centre;
  const float m_startAngleInRadians;
  const float m_endAngleInRadians;
  const HypotrochoidParams m_hypotrochoidParams;
  const float m_rDiff;
  const float m_numCusps;
  [[nodiscard]] static auto GetNumCusps(float bigR, float smallR) -> float;
  [[nodiscard]] auto GetPointAtAngle(float angle) const -> Point2dFlt;
};

class EpicycloidFunction : public IParametricFunction2d
{
public:
  struct EpicycloidParams
  {
    float k;
    float smallR;
    float amplitude;
  };
  struct Params
  {
    Point2dInt centrePos;
    AngleParams angleParams;
    EpicycloidParams epicycloidParams;
  };

  explicit EpicycloidFunction(const Params& params) noexcept;

  [[nodiscard]] auto GetPoint(float t) const -> Point2dFlt override;

private:
  const Vec2dFlt m_centre;
  const float m_startAngleInRadians;
  const float m_endAngleInRadians;
  const EpicycloidParams m_epicycloidParams;
  const float m_numCusps;
  [[nodiscard]] static auto GetNumCusps(float k) -> float;
  [[nodiscard]] auto GetPointAtAngle(float angle) const -> Point2dFlt;
};

class SineFunction : public IParametricFunction2d
{
public:
  struct Params
  {
    Point2dInt startPos;
    Point2dInt endPos;
    float amplitude = 1.0;
    float freq = 1.0;
  };

  explicit SineFunction(const Params& params) noexcept;

  [[nodiscard]] auto GetPoint(float t) const -> Point2dFlt override;

private:
  const Params m_params;
  const float m_distance;
  const float m_rotateAngle;
};

class OscillatingFunction : public IParametricFunction2d
{
public:
  struct Params
  {
    Point2dInt startPos;
    Point2dInt endPos;
    float oscillatingAmplitude = 1.0;
    float xOscillatingFreq = 1.0;
    float yOscillatingFreq = 1.0;
    bool allowOscillatingPath;
  };

  explicit OscillatingFunction(const Params& params);

  auto SetParams(const Params& params) -> void;
  auto SetAllowOscillatingPath(bool val) -> void;

  [[nodiscard]] auto GetPoint(float t) const -> Point2dFlt override;

private:
  Params m_params;
  [[nodiscard]] auto GetOscillatingPoint(const Point2dFlt& linearPoint, float t) const
      -> Point2dFlt;
};

inline LineFunction::LineFunction(const Params& params) noexcept
  : m_startPos{params.startPos.ToFlt()}, m_endPos{params.endPos.ToFlt()}
{
}

inline auto LineFunction::GetPoint(const float t) const -> Point2dFlt
{
  return lerp(m_startPos, m_endPos, t);
}

inline auto CircleFunction::GetPoint(const float t) const -> Point2dFlt
{
  const float currentAngle = STD20::lerp(m_startAngleInRadians, m_endAngleInRadians, t);
  return GetPointAtAngle(currentAngle);
}

inline auto LissajousFunction::GetPoint(const float t) const -> Point2dFlt
{
  const float currentAngle = STD20::lerp(m_startAngleInRadians, m_endAngleInRadians, t);
  return GetPointAtAngle(currentAngle);
}

inline auto HypotrochoidFunction::GetPoint(const float t) const -> Point2dFlt
{
  const float currentAngle = STD20::lerp(m_startAngleInRadians, m_endAngleInRadians, t);
  return GetPointAtAngle(m_numCusps * currentAngle);
}

inline auto EpicycloidFunction::GetPoint(const float t) const -> Point2dFlt
{
  const float currentAngle = STD20::lerp(m_startAngleInRadians, m_endAngleInRadians, t);
  return GetPointAtAngle(m_numCusps * currentAngle);
}

inline auto OscillatingFunction::SetAllowOscillatingPath(const bool val) -> void
{
  m_params.allowOscillatingPath = val;
}

inline auto OscillatingFunction::SetParams(const Params& params) -> void
{
  m_params = params;
}

} // namespace GOOM::UTILS::MATH
