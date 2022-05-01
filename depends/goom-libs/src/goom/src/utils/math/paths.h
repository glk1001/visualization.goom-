#pragma once

#include "misc.h"
#include "point2d.h"
#include "utils/math/transform2d.h"
#include "utils/t_values.h"

#include <cstdint>
#include <memory>

namespace GOOM::UTILS::MATH
{

class IPath
{
public:
  IPath() noexcept = default;
  IPath(const IPath&) noexcept = delete;
  IPath(IPath&&) noexcept = default;
  virtual ~IPath() noexcept = default;
  auto operator=(const IPath&) -> IPath& = delete;
  auto operator=(IPath&&) -> IPath& = delete;

  [[nodiscard]] virtual auto GetStartPos() const -> Point2dInt;
  [[nodiscard]] virtual auto GetEndPos() const -> Point2dInt;

  [[nodiscard]] virtual auto GetPositionT() const -> const TValue& = 0;

  [[nodiscard]] auto GetNumSteps() const -> uint32_t;
  virtual auto SetStepSize(float val) noexcept -> void = 0;

  [[nodiscard]] auto GetStepSize() const -> float;
  virtual auto SetNumSteps(uint32_t val) noexcept -> void = 0;

  [[nodiscard]] auto GetCurrentT() const -> float;
  [[nodiscard]] auto IsStopped() const -> bool;

  virtual auto IncrementT() -> void = 0;
  virtual auto Reset(float t) -> void = 0;
  [[nodiscard]] virtual auto GetNextPoint() const -> Point2dInt = 0;

private:
  friend class TransformedPath;
  friend class LerpedPath;
};

class IStandardPath : public IPath
{
public:
  explicit IStandardPath(std::unique_ptr<TValue> positionT) noexcept;
  IStandardPath(const IStandardPath&) noexcept = delete;
  IStandardPath(IStandardPath&&) noexcept = default;
  ~IStandardPath() noexcept override = default;
  auto operator=(const IStandardPath&) -> IStandardPath& = delete;
  auto operator=(IStandardPath&&) -> IStandardPath& = delete;

  [[nodiscard]] auto GetPositionT() const -> const TValue& override;

  auto SetStepSize(float val) noexcept -> void override;
  auto SetNumSteps(uint32_t val) noexcept -> void override;

  auto IncrementT() -> void override;
  auto Reset(float t) -> void override;

private:
  std::unique_ptr<TValue> m_positionT;
};

class IPathWithStartAndEnd : public IStandardPath
{
public:
  IPathWithStartAndEnd(const Point2dInt& startPos,
                       const Point2dInt& endPos,
                       std::unique_ptr<TValue> positionT) noexcept;

  [[nodiscard]] auto GetStartPos() const -> Point2dInt override;
  [[nodiscard]] auto GetEndPos() const -> Point2dInt override;

  auto SetStartPos(const Point2dInt& startPos) noexcept -> void;
  auto SetEndPos(const Point2dInt& endPos) noexcept -> void;

private:
  Point2dInt m_startPos;
  Point2dInt m_endPos;
};

class TransformedPath : public IPath
{
public:
  TransformedPath(std::shared_ptr<IPath> path, const MATH::Transform2d& transform);

  [[nodiscard]] auto GetStartPos() const -> Point2dInt override;
  [[nodiscard]] auto GetEndPos() const -> Point2dInt override;

  [[nodiscard]] auto GetPositionT() const -> const TValue& override;

  auto SetStepSize(float val) noexcept -> void override;
  auto SetNumSteps(uint32_t val) noexcept -> void override;

  auto IncrementT() -> void override;
  [[nodiscard]] auto GetNextPoint() const -> Point2dInt override;
  auto Reset(float t) -> void override;

private:
  std::shared_ptr<IPath> m_path;
  const MATH::Transform2d m_transform;
};

class LerpedPath : public IPath
{
public:
  LerpedPath(std::shared_ptr<IPath> path1, std::shared_ptr<IPath> path2, TValue& lerpT);

  [[nodiscard]] auto GetStartPos() const -> Point2dInt override;
  [[nodiscard]] auto GetEndPos() const -> Point2dInt override;

  auto IncrementT() -> void override;
  [[nodiscard]] auto GetNextPoint() const -> Point2dInt override;

private:
  std::shared_ptr<IPath> m_path1;
  std::shared_ptr<IPath> m_path2;
  TValue& m_lerpT;
};

class LinearPath : public IPathWithStartAndEnd
{
public:
  using IPathWithStartAndEnd::IPathWithStartAndEnd;

  [[nodiscard]] auto GetNextPoint() const -> Point2dInt override;
};

struct AngleParams
{
  float startAngleInDegrees = 0.0F;
  float endAngleInDegrees = DEGREES_360;
};

class CirclePath : public IStandardPath
{
public:
  CirclePath(const Point2dInt& centrePos,
             std::unique_ptr<TValue> positionT,
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

class LissajousPath : public IStandardPath
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
                std::unique_ptr<TValue> positionT,
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

class Hypotrochoid : public IStandardPath
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
               std::unique_ptr<TValue> positionT,
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

class Epicycloid : public IStandardPath
{
public:
  struct Params
  {
    float k;
    float smallR;
    float amplitude;
  };
  Epicycloid(const Point2dInt& centrePos,
             std::unique_ptr<TValue> positionT,
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

class SinePath : public IPathWithStartAndEnd
{
public:
  struct Params
  {
    float amplitude = 1.0;
    float freq = 1.0;
  };

  SinePath(const Point2dInt& startPos,
           const Point2dInt& endPos,
           std::unique_ptr<TValue> positionT,
           const Params& params) noexcept;

  [[nodiscard]] auto GetNextPoint() const -> Point2dInt override;

private:
  const Params m_params;
  const float m_distance;
  const float m_rotateAngle;
};

class OscillatingPath : public IPathWithStartAndEnd
{
public:
  struct Params
  {
    float oscillatingAmplitude = 1.0;
    float xOscillatingFreq = 1.0;
    float yOscillatingFreq = 1.0;
  };

  OscillatingPath(const Point2dInt& startPos,
                  const Point2dInt& endPos,
                  std::unique_ptr<TValue> positionT,
                  const Params& params,
                  bool allowOscillatingPath);

  auto SetParams(const Params& params) -> void;
  auto SetAllowOscillatingPath(bool val) -> void;

  [[nodiscard]] auto GetNextPoint() const -> Point2dInt override;

private:
  Params m_params;
  bool m_allowOscillatingPath;
  [[nodiscard]] auto GetOscillatingPointAtNextT(const Point2dFlt& linearPoint) const -> Point2dFlt;
};

inline auto IPath::GetStartPos() const -> Point2dInt
{
  return {};
}

inline auto IPath::GetEndPos() const -> Point2dInt
{
  return {};
}

inline auto IPath::GetNumSteps() const -> uint32_t
{
  return GetPositionT().GetNumSteps();
}

inline auto IPath::GetStepSize() const -> float
{
  return GetPositionT().GetStepSize();
}

inline auto IPath::GetCurrentT() const -> float
{
  return GetPositionT()();
}

inline auto IPath::IsStopped() const -> bool
{
  return GetPositionT().IsStopped();
}

inline IStandardPath::IStandardPath(std::unique_ptr<TValue> positionT) noexcept
  : m_positionT{std::move(positionT)}
{
}

inline auto IStandardPath::GetPositionT() const -> const TValue&
{
  return *m_positionT;
}

inline auto IStandardPath::SetStepSize(const float val) noexcept -> void
{
  m_positionT->SetStepSize(val);
}

inline auto IStandardPath::SetNumSteps(const uint32_t val) noexcept -> void
{
  m_positionT->SetNumSteps(val);
}

inline auto IStandardPath::IncrementT() -> void
{
  m_positionT->Increment();
}

inline auto IStandardPath::Reset(const float t) -> void
{
  m_positionT->Reset(t);
}

inline IPathWithStartAndEnd::IPathWithStartAndEnd(const Point2dInt& startPos,
                                                  const Point2dInt& endPos,
                                                  std::unique_ptr<TValue> positionT) noexcept
  : IStandardPath{std::move(positionT)}, m_startPos{startPos}, m_endPos{endPos}
{
}

inline auto IPathWithStartAndEnd::GetStartPos() const -> Point2dInt
{
  return m_startPos;
}

inline auto IPathWithStartAndEnd::GetEndPos() const -> Point2dInt
{
  return m_endPos;
}

inline auto IPathWithStartAndEnd::SetStartPos(const Point2dInt& startPos) noexcept -> void
{
  m_startPos = startPos;
}

inline auto IPathWithStartAndEnd::SetEndPos(const Point2dInt& endPos) noexcept -> void
{
  m_endPos = endPos;
}

inline auto TransformedPath::GetStartPos() const -> Point2dInt
{
  return m_transform.GetTransformedPoint(m_path->GetStartPos());
}

inline auto TransformedPath::GetEndPos() const -> Point2dInt
{
  return m_transform.GetTransformedPoint(m_path->GetEndPos());
}

inline auto TransformedPath::GetNextPoint() const -> Point2dInt
{
  return m_transform.GetTransformedPoint(m_path->GetNextPoint());
}

inline auto TransformedPath::GetPositionT() const -> const TValue&
{
  return m_path->GetPositionT();
}

inline auto TransformedPath::SetStepSize(const float val) noexcept -> void
{
  m_path->SetStepSize(val);
}

inline auto TransformedPath::SetNumSteps(const uint32_t val) noexcept -> void
{
  m_path->SetNumSteps(val);
}

inline auto TransformedPath::IncrementT() -> void
{
  m_path->IncrementT();
}

inline auto TransformedPath::Reset(const float t) -> void
{
  m_path->Reset(t);
}

inline auto LerpedPath::GetStartPos() const -> Point2dInt
{
  return lerp(m_path1->GetStartPos(), m_path2->GetStartPos(), 0.0F);
}

inline auto LerpedPath::GetEndPos() const -> Point2dInt
{
  return lerp(m_path1->GetEndPos(), m_path2->GetEndPos(), 1.0F);
}

inline auto LerpedPath::GetNextPoint() const -> Point2dInt
{
  return lerp(m_path1->GetNextPoint(), m_path2->GetNextPoint(), m_lerpT());
}

inline auto LerpedPath::IncrementT() -> void
{
  m_path1->IncrementT();
  m_path2->IncrementT();
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

inline auto OscillatingPath::SetAllowOscillatingPath(const bool val) -> void
{
  m_allowOscillatingPath = val;
}

inline auto OscillatingPath::SetParams(const Params& params) -> void
{
  m_params = params;
}

} // namespace GOOM::UTILS::MATH
