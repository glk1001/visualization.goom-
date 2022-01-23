#pragma once

#include "../v2d.h"
#include "t_values.h"

#include <cmath>
#include <cstdint>

namespace GOOM::UTILS
{

class IPath
{
public:
  IPath(const V2dInt& startPos, const V2dInt& finishPos) noexcept;
  IPath(const IPath&) noexcept = delete;
  IPath(IPath&&) noexcept = delete;
  auto operator=(const IPath&) -> IPath& = delete;
  auto operator=(IPath&&) -> IPath& = delete;
  virtual ~IPath() = default;

  [[nodiscard]] auto GetStartPos() const -> V2dInt;
  [[nodiscard]] auto GetFinishPos() const -> V2dInt;

  [[nodiscard]] virtual auto GetNextPoint() const -> V2dInt = 0;

private:
  const V2dInt m_startPos;
  const V2dInt m_finishPos;
};

class LinearTimePath : public IPath
{
public:
  LinearTimePath(const V2dInt& startPos, const V2dInt& finishPos, const TValue& t) noexcept;

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
  OscillatingPath(const V2dInt& startPos,
                  const V2dInt& finishPos,
                  const TValue& t,
                  bool allowOscillatingPath);

  void SetPathParams(const PathParams& params);
  void SetAllowOscillatingPath(bool val);

  [[nodiscard]] auto GetNextPoint() const -> V2dInt override;

private:
  V2dInt m_currentStartPos;
  V2dInt m_currentFinishPos;
  PathParams m_pathParams{};
  bool m_allowOscillatingPath;
  [[nodiscard]] auto GetPointAtNextT(const V2dInt& point0, const V2dInt& point1) const -> V2dInt;
  [[nodiscard]] auto GetOscillatingPointAtNextT(const V2dFlt& point) const -> V2dFlt;
};

inline IPath::IPath(const V2dInt& startPos, const V2dInt& finishPos) noexcept
  : m_startPos{startPos}, m_finishPos{finishPos}
{
}

inline auto IPath::GetStartPos() const -> V2dInt
{
  return m_startPos;
}

inline auto IPath::GetFinishPos() const -> V2dInt
{
  return m_finishPos;
}

inline LinearTimePath::LinearTimePath(const V2dInt& startPos,
                                      const V2dInt& finishPos,
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

inline OscillatingPath::OscillatingPath(const V2dInt& startPos,
                                        const V2dInt& finishPos,
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

inline auto OscillatingPath::GetNextPoint() const -> V2dInt
{
  return GetPointAtNextT(m_currentStartPos, m_currentFinishPos);
}

inline auto OscillatingPath::GetPointAtNextT(const V2dInt& point0, const V2dInt& point1) const
    -> V2dInt
{
  const V2dFlt linearPoint = lerp(point0.ToFlt(), point1.ToFlt(), GetT());

  if (!m_allowOscillatingPath)
  {
    return {static_cast<int32_t>(std::round(linearPoint.x)),
            static_cast<int32_t>(std::round(linearPoint.y))};
  }

  const V2dFlt finalPoint = GetOscillatingPointAtNextT(linearPoint);
  return {static_cast<int32_t>(std::round(finalPoint.x)),
          static_cast<int32_t>(std::round(finalPoint.y))};
}

inline auto OscillatingPath::GetOscillatingPointAtNextT(const V2dFlt& point) const -> V2dFlt
{
  return {
      point.x + (m_pathParams.oscillatingAmplitude *
                 std::cos(m_pathParams.xOscillatingFreq * GetT() * m_two_pi)),
      point.y + (m_pathParams.oscillatingAmplitude *
                 std::sin(m_pathParams.yOscillatingFreq * GetT() * m_two_pi)),
  };
}

} // namespace GOOM::UTILS
