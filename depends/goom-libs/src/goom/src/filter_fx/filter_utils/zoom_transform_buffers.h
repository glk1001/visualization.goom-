#pragma once

#include "goom_types.h"
#include "point2d.h"
#include "utils/math/misc.h"
#include "zoom_coord_transforms.h"

#include <algorithm>
#include <cstdint>
#include <vector>

namespace GOOM::FILTER_FX::FILTER_UTILS
{

class ZoomTransformBuffers
{
  static constexpr auto MAX_TRAN_LERP_EXP = 16U;

public:
  static constexpr auto MAX_TRAN_LERP_VALUE = UTILS::MATH::PowerOf2(MAX_TRAN_LERP_EXP);

  ZoomTransformBuffers(const Dimensions& dimensions, const Point2dInt& maxTranPoint) noexcept;

  auto SetSrceTranToIdentity() noexcept -> void;
  auto CopyDestTranToSrceTran() noexcept -> void;
  auto SwapDestTran(std::vector<Point2dInt>& otherTran) noexcept -> void;

  [[nodiscard]] auto GetTranSrce(size_t buffPos) const noexcept -> Point2dInt;
  [[nodiscard]] auto GetTranDest(size_t buffPos) const noexcept -> Point2dInt;

  [[nodiscard]] auto GetTranLerpFactor() const noexcept -> uint32_t;
  auto SetTranLerpFactor(uint32_t val) noexcept -> void;

  [[nodiscard]] auto GetSrceDestLerpBufferPoint(size_t buffPos, bool& isClipped) const noexcept
      -> Point2dInt;

  [[nodiscard]] static auto GetTranBuffLerpPoint(const Point2dInt& srcePoint,
                                                 const Point2dInt& destPoint,
                                                 uint32_t t) noexcept -> Point2dInt;

private:
  Dimensions m_dimensions;
  uint32_t m_bufferSize = m_dimensions.GetSize();
  Point2dInt m_maxTranPointMinus1;
  std::vector<Point2dInt> m_tranSrce;
  std::vector<Point2dInt> m_tranDest;
  uint32_t m_tranLerpFactor = 0U;

  [[nodiscard]] auto GetSrceDestLerpBufferPoint(size_t buffPos) const noexcept -> Point2dInt;
  struct SrceDestVals
  {
    int32_t srceBuffVal;
    int32_t destBuffVal;
  };
  [[nodiscard]] static auto GetTranBuffLerpVal(const SrceDestVals& srceDestVals,
                                               uint32_t t) noexcept -> int32_t;
  [[nodiscard]] auto GetClampedXVal(int32_t x) const noexcept -> int32_t;
  [[nodiscard]] auto GetClampedYVal(int32_t y) const noexcept -> int32_t;
};

inline auto ZoomTransformBuffers::GetTranSrce(const size_t buffPos) const noexcept -> Point2dInt
{
  return m_tranSrce[buffPos];
}

inline auto ZoomTransformBuffers::GetTranDest(const size_t buffPos) const noexcept -> Point2dInt
{
  return m_tranDest[buffPos];
}

inline auto ZoomTransformBuffers::SwapDestTran(std::vector<Point2dInt>& otherTran) noexcept -> void
{
  std::swap(m_tranDest, otherTran);
}

inline auto ZoomTransformBuffers::GetTranLerpFactor() const noexcept -> uint32_t
{
  return m_tranLerpFactor;
}

inline auto ZoomTransformBuffers::SetTranLerpFactor(const uint32_t val) noexcept -> void
{
  m_tranLerpFactor = val;
}

inline auto ZoomTransformBuffers::GetSrceDestLerpBufferPoint(const size_t buffPos) const noexcept
    -> Point2dInt
{
  auto isClipped = false;
  return GetSrceDestLerpBufferPoint(buffPos, isClipped);
}

inline auto ZoomTransformBuffers::GetSrceDestLerpBufferPoint(const size_t buffPos,
                                                             bool& isClipped) const noexcept
    -> Point2dInt
{
  const auto lerpPoint =
      GetTranBuffLerpPoint(m_tranSrce[buffPos], m_tranDest[buffPos], m_tranLerpFactor);

  if (lerpPoint.x < 0)
  {
    isClipped = true;
    return {0, GetClampedYVal(lerpPoint.y)};
  }
  if (lerpPoint.x > m_maxTranPointMinus1.x)
  {
    isClipped = true;
    return {m_maxTranPointMinus1.x, GetClampedYVal(lerpPoint.y)};
  }

  if (lerpPoint.y < 0)
  {
    isClipped = true;
    return {GetClampedXVal(lerpPoint.x), 0};
  }
  if (lerpPoint.y > m_maxTranPointMinus1.y)
  {
    isClipped = true;
    return {GetClampedXVal(lerpPoint.x), m_maxTranPointMinus1.y};
  }

  return lerpPoint;
}

inline auto ZoomTransformBuffers::GetClampedXVal(const int32_t x) const noexcept -> int32_t
{
  return std::clamp(x, 0, m_maxTranPointMinus1.x);
}

inline auto ZoomTransformBuffers::GetClampedYVal(const int32_t y) const noexcept -> int32_t
{
  return std::clamp(y, 0, m_maxTranPointMinus1.y);
}

inline auto ZoomTransformBuffers::GetTranBuffLerpPoint(const Point2dInt& srcePoint,
                                                       const Point2dInt& destPoint,
                                                       const uint32_t t) noexcept -> Point2dInt
{
  return {GetTranBuffLerpVal({srcePoint.x, destPoint.x}, t),
          GetTranBuffLerpVal({srcePoint.y, destPoint.y}, t)};
}

inline auto ZoomTransformBuffers::GetTranBuffLerpVal(const SrceDestVals& srceDestVals,
                                                     const uint32_t t) noexcept -> int32_t
{
  const auto diff      = static_cast<int64_t>(srceDestVals.destBuffVal - srceDestVals.srceBuffVal);
  const auto numerator = static_cast<int64_t>(t) * diff;
  const auto result    = static_cast<int32_t>(static_cast<int64_t>(srceDestVals.srceBuffVal) +
                                           (numerator >> MAX_TRAN_LERP_EXP));

  static constexpr auto HALF_MAX_TRAN_LERP_VALUE = MAX_TRAN_LERP_VALUE / 2;
  if (const auto mod = numerator & static_cast<int64_t>(MAX_TRAN_LERP_VALUE - 1U);
      mod >= static_cast<int64_t>(HALF_MAX_TRAN_LERP_VALUE))
  {
    return result + 1;
  }
  return result;
}

} // namespace GOOM::FILTER_FX::FILTER_UTILS
