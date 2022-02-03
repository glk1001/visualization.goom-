#pragma once

#include "../point2d.h"
#include "t_values.h"

#include <cmath>
#include <cstdint>

namespace GOOM::UTILS
{

class IPath
{
public:
  IPath(const Point2dInt& startPos, const Point2dInt& finishPos) noexcept;
  IPath(const IPath&) noexcept = delete;
  IPath(IPath&&) noexcept = delete;
  auto operator=(const IPath&) -> IPath& = delete;
  auto operator=(IPath&&) -> IPath& = delete;
  virtual ~IPath() = default;

  [[nodiscard]] auto GetStartPos() const -> Point2dInt;
  [[nodiscard]] auto GetFinishPos() const -> Point2dInt;

  [[nodiscard]] virtual auto GetNextPoint() const -> Point2dInt = 0;

private:
  const Point2dInt m_startPos;
  const Point2dInt m_finishPos;
};

class LinearTimePath : public IPath
{
public:
  LinearTimePath(const Point2dInt& startPos, const Point2dInt& finishPos, const TValue& t) noexcept;

  [[nodiscard]] auto GetStepSize() const -> float;
  [[nodiscard]] auto GetT() const -> float;

private:
  const TValue& m_t;
};

struct PathParams
{
  float oscillatingAmplitude = 1.0;
  float xOscillatingFreq = 1.0;
  float yOscillatingFreq = 1.0;
};

class OscillatingPath : public LinearTimePath
{
public:
  OscillatingPath(const Point2dInt& startPos,
                  const Point2dInt& finishPos,
                  const TValue& t,
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

inline IPath::IPath(const Point2dInt& startPos, const Point2dInt& finishPos) noexcept
  : m_startPos{startPos}, m_finishPos{finishPos}
{
}

inline auto IPath::GetStartPos() const -> Point2dInt
{
  return m_startPos;
}

inline auto IPath::GetFinishPos() const -> Point2dInt
{
  return m_finishPos;
}

inline LinearTimePath::LinearTimePath(const Point2dInt& startPos,
                                      const Point2dInt& finishPos,
                                      const TValue& t) noexcept
  : IPath{startPos, finishPos}, m_t{t}
{
}

inline auto LinearTimePath::GetStepSize() const -> float
{
  return m_t.GetStepSize();
}

inline auto LinearTimePath::GetT() const -> float
{
  return m_t();
}

inline OscillatingPath::OscillatingPath(const Point2dInt& startPos,
                                        const Point2dInt& finishPos,
                                        const TValue& t,
                                        const bool allowOscillatingPath)
  : LinearTimePath{startPos, finishPos, t},
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
  const Point2dFlt linearPoint = lerp(point0.ToFlt(), point1.ToFlt(), GetT());

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
                 std::cos(m_pathParams.xOscillatingFreq * GetT() * m_two_pi)),
      point.y + (m_pathParams.oscillatingAmplitude *
                 std::sin(m_pathParams.yOscillatingFreq * GetT() * m_two_pi)),
  };
}

} // namespace GOOM::UTILS
