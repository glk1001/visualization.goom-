#pragma once

#include "misc.h"
#include "point2d.h"
#include "utils/t_values.h"

#include <cstdint>

namespace GOOM::UTILS::MATH
{

class IPath
{
public:
  explicit IPath(TValue& positionT) noexcept;
  IPath(const Point2dInt& startPos, const Point2dInt& endPos, TValue& positionT) noexcept;
  IPath(const IPath&) noexcept = default;
  IPath(IPath&&) noexcept = default;
  virtual ~IPath() noexcept = default;
  auto operator=(const IPath&) -> IPath& = delete;
  auto operator=(IPath&&) -> IPath& = delete;

  [[nodiscard]] virtual auto GetStartPos() const -> Point2dInt;
  [[nodiscard]] virtual auto GetEndPos() const -> Point2dInt;

  [[nodiscard]] auto GetNumSteps() const -> uint32_t;
  [[nodiscard]] auto GetStepSize() const -> float;
  [[nodiscard]] auto GetCurrentT() const -> float;
  [[nodiscard]] auto IsStopped() const -> bool;

  void IncrementT();
  void Reset(float t = 0.0);

  [[nodiscard]] virtual auto GetNextPoint() const -> Point2dInt = 0;

private:
  const Point2dInt m_startPos;
  const Point2dInt m_endPos;
  TValue& m_positionT;
  friend class LerpedPath;
};

class LerpedPath : public IPath
{
public:
  LerpedPath(TValue& positionT, IPath& path1, IPath& path2, TValue& lerpT);

  [[nodiscard]] auto GetStartPos() const -> Point2dInt override;
  [[nodiscard]] auto GetEndPos() const -> Point2dInt override;
  [[nodiscard]] auto GetNextPoint() const -> Point2dInt override;

private:
  IPath& m_path1;
  IPath& m_path2;
  TValue& m_lerpT;
};

class LinearPath : public IPath
{
public:
  using IPath::IPath;

  [[nodiscard]] auto GetNextPoint() const -> Point2dInt override;
};

struct AngleParams
{
  float startAngleInDegrees = 0.0F;
  float endAngleInDegrees = DEGREES_360;
};

class CirclePath : public IPath
{
public:
  CirclePath(const Point2dInt& centrePos,
             TValue& positionT,
             float radius,
             const AngleParams& angleParams = AngleParams{}) noexcept;

  [[nodiscard]] auto GetStartPos() const -> Point2dInt override;
  [[nodiscard]] auto GetEndPos() const -> Point2dInt override;
  [[nodiscard]] auto GetNextPoint() const -> Point2dInt override;

private:
  const Vec2dFlt m_centre;
  const float m_radius;
  const float m_startAngleInRadians;
  const float m_endAngleInRadians;
  [[nodiscard]] auto GetPoint(float angle) const -> Point2dFlt;
};

class LissajousPath : public IPath
{
public:
  struct Params
  {
    float a;
    float b;
    float kX;
    float kY;
  };

  LissajousPath(const Point2dInt& centrePos,
                TValue& positionT,
                const Params& params,
                const AngleParams& angleParams = AngleParams{}) noexcept;

  [[nodiscard]] auto GetStartPos() const -> Point2dInt override;
  [[nodiscard]] auto GetEndPos() const -> Point2dInt override;
  [[nodiscard]] auto GetNextPoint() const -> Point2dInt override;

private:
  const Vec2dFlt m_centre;
  const Params m_params;
  const float m_startAngleInRadians;
  const float m_endAngleInRadians;
  [[nodiscard]] auto GetPoint(float angle) const -> Point2dFlt;
};

class Hypotrochoid : public IPath
{
public:
  struct Params
  {
    float bigR;
    float smallR;
    float height;
    float amplitude;
  };
  Hypotrochoid(const Point2dInt& centrePos,
               TValue& positionT,
               const Params& params,
               const AngleParams& angleParams = AngleParams{}) noexcept;

  [[nodiscard]] auto GetStartPos() const -> Point2dInt override;
  [[nodiscard]] auto GetEndPos() const -> Point2dInt override;
  [[nodiscard]] auto GetNextPoint() const -> Point2dInt override;

private:
  const Vec2dFlt m_centre;
  const Params m_params;
  const float m_startAngleInRadians;
  const float m_endAngleInRadians;
  const float m_rDiff;
  const float m_numCusps;
  [[nodiscard]] static auto GetNumCusps(float bigR, float smallR) -> float;
  [[nodiscard]] auto GetPoint(float angle) const -> Point2dFlt;
};

class Epicycloid : public IPath
{
public:
  struct Params
  {
    float k;
    float smallR;
    float amplitude;
  };
  Epicycloid(const Point2dInt& centrePos,
             TValue& positionT,
             const Params& params,
             const AngleParams& angleParams = AngleParams{}) noexcept;

  [[nodiscard]] auto GetStartPos() const -> Point2dInt override;
  [[nodiscard]] auto GetEndPos() const -> Point2dInt override;
  [[nodiscard]] auto GetNextPoint() const -> Point2dInt override;

private:
  const Vec2dFlt m_centre;
  const Params m_params;
  const float m_startAngleInRadians;
  const float m_endAngleInRadians;
  const float m_numCusps;
  [[nodiscard]] static auto GetNumCusps(float k) -> float;
  [[nodiscard]] auto GetPoint(float angle) const -> Point2dFlt;
};

class SinePath : public IPath
{
public:
  struct Params
  {
    float amplitude = 1.0;
    float freq = 1.0;
  };

  SinePath(const Point2dInt& startPos,
           const Point2dInt& endPos,
           TValue& positionT,
           const Params& params) noexcept;

  [[nodiscard]] auto GetNextPoint() const -> Point2dInt override;

private:
  const Params m_params;
  const float m_distance;
  const float m_rotateAngle;
};

class OscillatingPath : public IPath
{
public:
  struct Params
  {
    float oscillatingAmplitude = 1.0;
    float xOscillatingFreq = 1.0;
    float yOscillatingFreq = 1.0;
  };

  OscillatingPath(const Point2dInt& startPos,
                  const Point2dInt& finishPos,
                  TValue& t,
                  const Params& params,
                  bool allowOscillatingPath);

  void SetParams(const Params& params);
  void SetAllowOscillatingPath(bool val);

  [[nodiscard]] auto GetNextPoint() const -> Point2dInt override;

private:
  Point2dInt m_currentStartPos;
  Point2dInt m_currentFinishPos;
  Params m_params;
  bool m_allowOscillatingPath;
  [[nodiscard]] auto GetPointAtNextT(const Point2dInt& point0, const Point2dInt& point1) const
      -> Point2dInt;
  [[nodiscard]] auto GetOscillatingPointAtNextT(const Point2dFlt& point) const -> Point2dFlt;
};

inline IPath::IPath(TValue& positionT) noexcept : m_startPos{}, m_endPos{}, m_positionT{positionT}
{
}

inline IPath::IPath(const Point2dInt& startPos,
                    const Point2dInt& endPos,
                    TValue& positionT) noexcept
  : m_startPos{startPos}, m_endPos{endPos}, m_positionT{positionT}
{
}

inline auto IPath::GetStartPos() const -> Point2dInt
{
  return m_startPos;
}

inline auto IPath::GetEndPos() const -> Point2dInt
{
  return m_endPos;
}

inline auto IPath::GetNumSteps() const -> uint32_t
{
  return m_positionT.GetNumSteps();
}

inline auto IPath::GetStepSize() const -> float
{
  return m_positionT.GetStepSize();
}

inline auto IPath::GetCurrentT() const -> float
{
  return m_positionT();
}

inline auto IPath::IsStopped() const -> bool
{
  return m_positionT.IsStopped();
}

inline void IPath::IncrementT()
{
  m_positionT.Increment();
}

inline void IPath::Reset(const float t)
{
  m_positionT.Reset(t);
}

inline auto LerpedPath::GetNextPoint() const -> Point2dInt
{
  return lerp(m_path1.GetNextPoint(), m_path2.GetNextPoint(), m_lerpT());
}

inline auto LerpedPath::GetStartPos() const -> Point2dInt
{
  return lerp(m_path1.GetStartPos(), m_path2.GetStartPos(), 0.0F);
}

inline auto LerpedPath::GetEndPos() const -> Point2dInt
{
  return lerp(m_path1.GetEndPos(), m_path2.GetEndPos(), 1.0F);
}

inline auto LinearPath::GetNextPoint() const -> Point2dInt
{
  return lerp(GetStartPos(), GetEndPos(), GetCurrentT());
}

inline auto CirclePath::GetStartPos() const -> Point2dInt
{
  return GetPoint(m_startAngleInRadians).ToInt();
}

inline auto CirclePath::GetEndPos() const -> Point2dInt
{
  return GetPoint(m_endAngleInRadians).ToInt();
}

inline auto CirclePath::GetNextPoint() const -> Point2dInt
{
  const float currentAngle = STD20::lerp(m_startAngleInRadians, m_endAngleInRadians, GetCurrentT());
  return GetPoint(currentAngle).ToInt();
}

inline auto LissajousPath::GetStartPos() const -> Point2dInt
{
  return GetPoint(m_startAngleInRadians).ToInt();
}

inline auto LissajousPath::GetEndPos() const -> Point2dInt
{
  return GetPoint(m_endAngleInRadians).ToInt();
}

inline auto LissajousPath::GetNextPoint() const -> Point2dInt
{
  const float currentAngle = STD20::lerp(m_startAngleInRadians, m_endAngleInRadians, GetCurrentT());
  return GetPoint(currentAngle).ToInt();
}

inline auto Hypotrochoid::GetStartPos() const -> Point2dInt
{
  return GetPoint(m_startAngleInRadians).ToInt();
}

inline auto Hypotrochoid::GetEndPos() const -> Point2dInt
{
  return GetPoint(m_endAngleInRadians).ToInt();
}

inline auto Hypotrochoid::GetNextPoint() const -> Point2dInt
{
  const float currentAngle = STD20::lerp(m_startAngleInRadians, m_endAngleInRadians, GetCurrentT());
  return GetPoint(m_numCusps * currentAngle).ToInt();
}

inline auto Epicycloid::GetStartPos() const -> Point2dInt
{
  return GetPoint(m_startAngleInRadians).ToInt();
}

inline auto Epicycloid::GetEndPos() const -> Point2dInt
{
  return GetPoint(m_endAngleInRadians).ToInt();
}

inline auto Epicycloid::GetNextPoint() const -> Point2dInt
{
  const float currentAngle = STD20::lerp(m_startAngleInRadians, m_endAngleInRadians, GetCurrentT());
  return GetPoint(m_numCusps * currentAngle).ToInt();
}

inline void OscillatingPath::SetAllowOscillatingPath(const bool val)
{
  m_allowOscillatingPath = val;
}

inline void OscillatingPath::SetParams(const Params& params)
{
  m_params = params;
}

inline auto OscillatingPath::GetNextPoint() const -> Point2dInt
{
  return GetPointAtNextT(m_currentStartPos, m_currentFinishPos);
}

} // namespace GOOM::UTILS::MATH
