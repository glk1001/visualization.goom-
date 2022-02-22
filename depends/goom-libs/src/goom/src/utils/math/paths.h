#pragma once

#include "misc.h"
#include "point2d.h"
#include "utils/t_values.h"

#include <cmath>
#include <cstdint>

namespace GOOM::UTILS::MATH
{

class IPath
{
public:
  IPath(const Point2dInt& startPos, const Point2dInt& endPos, TValue& positionT) noexcept;
  IPath(const IPath&) noexcept = delete;
  IPath(IPath&&) noexcept = delete;
  virtual ~IPath() noexcept = default;
  auto operator=(const IPath&) -> IPath& = delete;
  auto operator=(IPath&&) -> IPath& = delete;

  [[nodiscard]] auto GetStartPos() const -> Point2dInt;
  [[nodiscard]] auto GetEndPos() const -> Point2dInt;

  [[nodiscard]] auto GetNumSteps() const -> uint32_t;
  [[nodiscard]] auto GetStepSize() const -> float;
  [[nodiscard]] auto GetCurrentT() const -> float;
  void IncrementT();

  [[nodiscard]] virtual auto GetNextPoint() const -> Point2dInt = 0;

private:
  const Point2dInt m_startPos;
  const Point2dInt m_endPos;
  TValue& m_positionT;
};


class LinearPath : public IPath
{
public:
  using IPath::IPath;

  [[nodiscard]] auto GetNextPoint() const -> Point2dInt override;
};

class SinePath : public IPath
{
public:
  SinePath(const Point2dInt& startPos, const Point2dInt& endPos, TValue& positionT) noexcept;

  [[nodiscard]] auto GetNextPoint() const -> Point2dInt override;

private:
  const float m_distance;
  const float m_rotateAngle;
};

struct PathParams
{
  float oscillatingAmplitude = 1.0;
  float xOscillatingFreq = 1.0;
  float yOscillatingFreq = 1.0;
};

class OscillatingPath : public IPath
{
public:
  OscillatingPath(const Point2dInt& startPos,
                  const Point2dInt& finishPos,
                  TValue& t,
                  bool allowOscillatingPath);

  void SetPathParams(const PathParams& params);
  void SetAllowOscillatingPath(bool val);

  [[nodiscard]] auto GetNextPoint() const -> Point2dInt override;

private:
  Point2dInt m_currentStartPos;
  Point2dInt m_currentFinishPos;
  PathParams m_pathParams{};
  bool m_allowOscillatingPath;
  [[nodiscard]] auto GetPointAtNextT(const Point2dInt& point0, const Point2dInt& point1) const
      -> Point2dInt;
  [[nodiscard]] auto GetOscillatingPointAtNextT(const Point2dFlt& point) const -> Point2dFlt;
};

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

inline void IPath::IncrementT()
{
  m_positionT.Increment();
}

inline auto LinearPath::GetNextPoint() const -> Point2dInt
{
  return lerp(GetStartPos(), GetEndPos(), GetCurrentT());
}

inline SinePath::SinePath(const Point2dInt& startPos,
                          const Point2dInt& endPos,
                          TValue& positionT) noexcept
  : IPath{startPos, endPos, positionT},
    m_distance{Distance(startPos.ToFlt(), endPos.ToFlt())},
    m_rotateAngle{std::asin((static_cast<float>(endPos.y - startPos.y)) / m_distance)}
{
}

inline auto SinePath::GetNextPoint() const -> Point2dInt
{
  constexpr float FREQ = 2.0F;

  const float y = 100.0F * std::sin(FREQ * TWO_PI * GetCurrentT());
  const float x = m_distance * GetCurrentT();

  Point2dFlt newPoint{x, y};
  newPoint.Rotate(m_rotateAngle);
  return (newPoint + Vec2dFlt{GetStartPos().ToFlt()}).ToInt();
}

inline OscillatingPath::OscillatingPath(const Point2dInt& startPos,
                                        const Point2dInt& finishPos,
                                        TValue& t,
                                        const bool allowOscillatingPath)
  : IPath{startPos, finishPos, t},
    m_currentStartPos{startPos},
    m_currentFinishPos{finishPos},
    m_allowOscillatingPath{allowOscillatingPath}
{
}

inline void OscillatingPath::SetAllowOscillatingPath(const bool val)
{
  m_allowOscillatingPath = val;
}

inline void OscillatingPath::SetPathParams(const PathParams& params)
{
  m_pathParams = params;
}

inline auto OscillatingPath::GetNextPoint() const -> Point2dInt
{
  return GetPointAtNextT(m_currentStartPos, m_currentFinishPos);
}

inline auto OscillatingPath::GetPointAtNextT(const Point2dInt& point0,
                                             const Point2dInt& point1) const -> Point2dInt
{
  const Point2dFlt linearPoint = lerp(point0.ToFlt(), point1.ToFlt(), GetCurrentT());

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
      point.x + (m_pathParams.oscillatingAmplitude *
                 std::cos(m_pathParams.xOscillatingFreq * GetCurrentT() * MATH::TWO_PI)),
      point.y + (m_pathParams.oscillatingAmplitude *
                 std::sin(m_pathParams.yOscillatingFreq * GetCurrentT() * MATH::TWO_PI)),
  };
}

} // namespace GOOM::UTILS::MATH
