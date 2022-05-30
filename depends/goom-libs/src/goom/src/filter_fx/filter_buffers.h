#pragma once

#include "goom_graphic.h"
#include "normalized_coords.h"
#include "point2d.h"
#include "utils/propagate_const.h"

#include <array>
#include <cmath>
#include <cstdint>
#include <functional>
#include <memory>
#include <vector>

namespace GOOM
{

class PluginInfo;
class PixelBuffer;

namespace UTILS
{
class Parallel;
}

namespace FILTER_FX
{

class ZoomFilterBuffers
{
public:
  static constexpr int32_t DIM_FILTER_COEFFS = 16;
  static constexpr size_t NUM_NEIGHBOR_COEFFS = 4;
  using NeighborhoodPixelArray = std::array<Pixel, NUM_NEIGHBOR_COEFFS>;
  static constexpr float MIN_SCREEN_COORD_ABS_VAL = 1.0F / static_cast<float>(DIM_FILTER_COEFFS);
  class CoordTransforms;

  enum class TranBuffersState
  {
    START_FRESH_TRAN_BUFFERS,
    RESET_TRAN_BUFFERS,
    TRAN_BUFFERS_READY,
  };

  using ZoomPointFunc = std::function<NormalizedCoords(const NormalizedCoords& normalizedCoords)>;

  ZoomFilterBuffers(UTILS::Parallel& parallel,
                    const PluginInfo& goomInfo,
                    const NormalizedCoordsConverter& normalizedCoordsConverter,
                    const ZoomPointFunc& zoomPointFunc) noexcept;

  [[nodiscard]] auto GetBuffMidpoint() const noexcept -> Point2dInt;
  auto SetBuffMidpoint(const Point2dInt& val) noexcept -> void;

  [[nodiscard]] auto GetTranLerpFactor() const noexcept -> int32_t;
  auto SetTranLerpFactor(int32_t val) noexcept -> void;
  [[nodiscard]] static auto GetMaxTranLerpFactor() noexcept -> int32_t;

  [[nodiscard]] auto GetTranBuffYLineStart() const noexcept -> uint32_t;

  auto Start() noexcept -> void;

  auto NotifyFilterSettingsHaveChanged() noexcept -> void;
  [[nodiscard]] auto HaveFilterSettingsChanged() const noexcept -> bool;

  auto UpdateTranBuffers() noexcept -> void;
  [[nodiscard]] auto GetTranBuffersState() const noexcept -> TranBuffersState;

  struct NeighborhoodCoeffArray
  {
    std::array<uint32_t, NUM_NEIGHBOR_COEFFS> val;
    bool isZero;
  };
  struct SourcePointInfo
  {
    Point2dInt screenPoint;
    NeighborhoodCoeffArray coeffs;
    bool isClipped;
  };
  [[nodiscard]] auto GetSourcePointInfo(size_t buffPos) const noexcept -> SourcePointInfo;

private:
  const uint32_t m_screenWidth;
  const uint32_t m_screenHeight;
  const NormalizedCoordsConverter& m_normalizedCoordsConverter;
  const std::unique_ptr<const CoordTransforms> m_coordTransforms{
      std::make_unique<CoordTransforms>(m_normalizedCoordsConverter)};

  class FilterCoefficients;
  const std::unique_ptr<const FilterCoefficients> m_precalculatedCoeffs{
      std::make_unique<FilterCoefficients>()};

  [[nodiscard]] auto GetMaxTranX() const noexcept -> uint32_t;
  [[nodiscard]] auto GetMaxTranY() const noexcept -> uint32_t;

  UTILS::Parallel& m_parallel;
  const ZoomPointFunc m_getZoomPoint;
  const Point2dInt m_maxTranPoint;
  const uint32_t m_tranBuffStripeHeight{m_screenHeight / DIM_FILTER_COEFFS};
  class TransformBuffers;
  std::experimental::propagate_const<std::unique_ptr<TransformBuffers>> m_transformBuffers{
      std::make_unique<TransformBuffers>(m_screenWidth, m_screenHeight, m_maxTranPoint)};

  Point2dInt m_buffMidpoint{0, 0};
  NormalizedCoords m_normalizedMidPt{0.0F, 0.0F};
  bool m_filterSettingsHaveChanged = false;

  uint32_t m_tranBuffYLineStart = 0;
  TranBuffersState m_tranBuffersState = TranBuffersState::TRAN_BUFFERS_READY;

  std::vector<int32_t> m_firedec;

  auto InitAllTranBuffers() noexcept -> void;
  auto StartFreshTranBuffers() noexcept -> void;
  auto ResetTranBuffers() noexcept -> void;
  auto FillTempTranBuffers() noexcept -> void;
  auto DoNextTempTranBuffersStripe(uint32_t tranBuffStripeHeight) noexcept -> void;
  auto GenerateWaterFxHorizontalBuffer() noexcept -> void;
  [[nodiscard]] auto GetZoomBufferTranPoint(size_t buffPos, bool& isClipped) const noexcept
      -> Point2dInt;
  [[nodiscard]] auto GetTranPoint(const NormalizedCoords& normalized) const noexcept -> Point2dInt;
};

class ZoomFilterBuffers::CoordTransforms
{
public:
  explicit CoordTransforms(const NormalizedCoordsConverter& normalizedCoordsConverter) noexcept;

  [[nodiscard]] auto NormalizedToTranPoint(const NormalizedCoords& normalizedPoint) const noexcept
      -> Point2dInt;

  // Use these consts for optimising multiplication, division, and mod, by DIM_FILTER_COEFFS.
  static constexpr int32_t MAX_TRAN_LERP_VALUE = 0xFFFF;
  static constexpr int32_t DIM_FILTER_COEFFS_DIV_SHIFT = 4;
  static constexpr int32_t DIM_FILTER_COEFFS_MOD_MASK = 0xF;

  [[nodiscard]] static auto TranCoordToCoeffIndex(uint32_t tranCoord) noexcept -> uint32_t;
  [[nodiscard]] static auto TranToScreenPoint(const Point2dInt& tranPoint) noexcept -> Point2dInt;
  [[nodiscard]] static auto ScreenToTranPoint(const Point2dInt& screenPoint) noexcept -> Point2dInt;
  [[nodiscard]] static auto ScreenToTranCoord(float screenCoord) noexcept -> uint32_t;

private:
  const NormalizedCoordsConverter& m_normalizedCoordsConverter;
};

class ZoomFilterBuffers::FilterCoefficients
{
public:
  FilterCoefficients() = default; // Android does not like 'noexcept'

  using FilterCoeff2dArray =
      std::array<std::array<NeighborhoodCoeffArray, DIM_FILTER_COEFFS>, DIM_FILTER_COEFFS>;
  [[nodiscard]] auto GetCoeffs() const noexcept -> const FilterCoeff2dArray&;

private:
  // modif d'optim by Jeko : precalcul des 4 coeffs resultant des 2 pos
  const FilterCoeff2dArray m_precalculatedCoeffs{GetPrecalculatedCoefficients()};
  [[nodiscard]] static auto GetPrecalculatedCoefficients() noexcept -> FilterCoeff2dArray;
  [[nodiscard]] static auto GetNeighborhoodCoeffArray(uint32_t coeffH, uint32_t coeffV) noexcept
      -> NeighborhoodCoeffArray;
};

class ZoomFilterBuffers::TransformBuffers
{
public:
  TransformBuffers(uint32_t screenWidth,
                   uint32_t screenHeight,
                   const Point2dInt& maxTranPoint) noexcept;

  auto SetSrceTranToIdentity() noexcept -> void;
  auto CopyTempTranToDestTran() noexcept -> void;
  auto CopyDestTranToSrceTran() noexcept -> void;
  auto SetUpNextDestTran() noexcept -> void;

  auto SetTempBuffersTransformPoint(uint32_t pos, const Point2dInt& transformPoint) noexcept
      -> void;

  [[nodiscard]] auto GetTranLerpFactor() const noexcept -> int32_t;
  auto SetTranLerpFactor(int32_t val) noexcept -> void;

  [[nodiscard]] auto GetSrceDestLerpBufferPoint(size_t buffPos, bool& isClipped) const noexcept
      -> Point2dInt;

private:
  const uint32_t m_screenWidth;
  const uint32_t m_screenHeight;
  const uint32_t m_bufferSize;
  const Point2dInt m_maxTranPointMinus1;
  std::vector<int32_t> m_tranXSrce{};
  std::vector<int32_t> m_tranYSrce{};
  std::vector<int32_t> m_tranXDest{};
  std::vector<int32_t> m_tranYDest{};
  std::vector<int32_t> m_tranXTemp{};
  std::vector<int32_t> m_tranYTemp{};
  int32_t m_tranLerpFactor = 0;

  auto CopyAllDestTranToSrceTran() noexcept -> void;
  auto CopyUnlerpedDestTranToSrceTran() noexcept -> void;
  [[nodiscard]] auto GetSrceDestLerpBufferPoint(size_t buffPos) const noexcept -> Point2dInt;
  [[nodiscard]] static auto GetTranBuffLerpVal(int32_t srceBuffVal,
                                               int32_t destBuffVal,
                                               int32_t t) noexcept -> int32_t;
  [[nodiscard]] auto GetClampedXVal(int32_t x) const noexcept -> int32_t;
  [[nodiscard]] auto GetClampedYVal(int32_t y) const noexcept -> int32_t;
};

inline auto ZoomFilterBuffers::CoordTransforms::TranCoordToCoeffIndex(
    const uint32_t tranCoord) noexcept -> uint32_t
{
  return tranCoord & DIM_FILTER_COEFFS_MOD_MASK;
}

inline auto ZoomFilterBuffers::CoordTransforms::TranToScreenPoint(
    const Point2dInt& tranPoint) noexcept -> Point2dInt
{
  return {tranPoint.x >> DIM_FILTER_COEFFS_DIV_SHIFT, tranPoint.y >> DIM_FILTER_COEFFS_DIV_SHIFT};
}

inline auto ZoomFilterBuffers::CoordTransforms::ScreenToTranPoint(
    const Point2dInt& screenPoint) noexcept -> Point2dInt
{
  return {screenPoint.x << DIM_FILTER_COEFFS_DIV_SHIFT,
          screenPoint.y << DIM_FILTER_COEFFS_DIV_SHIFT};
}

inline auto ZoomFilterBuffers::CoordTransforms::ScreenToTranCoord(const float screenCoord) noexcept
    -> uint32_t
{
  // IMPORTANT: Without 'lround' a faint cross artifact appears in the centre of the screen.
  return static_cast<uint32_t>(std::lround(screenCoord * static_cast<float>(DIM_FILTER_COEFFS)));
}

inline ZoomFilterBuffers::CoordTransforms::CoordTransforms(
    const NormalizedCoordsConverter& normalizedCoordsConverter) noexcept
  : m_normalizedCoordsConverter{normalizedCoordsConverter}
{
}

inline auto ZoomFilterBuffers::CoordTransforms::NormalizedToTranPoint(
    const NormalizedCoords& normalizedPoint) const noexcept -> Point2dInt
{
  const Point2dFlt screenCoords =
      m_normalizedCoordsConverter.NormalizedToScreenCoordsFlt(normalizedPoint);

  // IMPORTANT: Without 'lround' a faint cross artifact appears in the centre of the screen.
  return {static_cast<int32_t>(std::lround(ScreenToTranCoord(screenCoords.x))),
          static_cast<int32_t>(std::lround(ScreenToTranCoord(screenCoords.y)))};
}

inline auto ZoomFilterBuffers::GetBuffMidpoint() const noexcept -> Point2dInt
{
  return m_buffMidpoint;
}

inline auto ZoomFilterBuffers::SetBuffMidpoint(const Point2dInt& val) noexcept -> void
{
  m_buffMidpoint = val;
  m_normalizedMidPt = m_normalizedCoordsConverter.ScreenToNormalizedCoords(m_buffMidpoint);
}

inline auto ZoomFilterBuffers::GetTranBuffersState() const noexcept -> TranBuffersState
{
  return m_tranBuffersState;
}

inline auto ZoomFilterBuffers::GetTranLerpFactor() const noexcept -> int32_t
{
  return m_transformBuffers->GetTranLerpFactor();
}

inline auto ZoomFilterBuffers::GetMaxTranLerpFactor() noexcept -> int32_t
{
  return CoordTransforms::MAX_TRAN_LERP_VALUE;
}

inline auto ZoomFilterBuffers::SetTranLerpFactor(const int32_t val) noexcept -> void
{
  m_transformBuffers->SetTranLerpFactor(val);
}

inline auto ZoomFilterBuffers::GetMaxTranX() const noexcept -> uint32_t
{
  return static_cast<uint32_t>(m_maxTranPoint.x);
}

inline auto ZoomFilterBuffers::GetMaxTranY() const noexcept -> uint32_t
{
  return static_cast<uint32_t>(m_maxTranPoint.y);
}

inline auto ZoomFilterBuffers::GetTranBuffYLineStart() const noexcept -> uint32_t
{
  return m_tranBuffYLineStart;
}

inline auto ZoomFilterBuffers::TransformBuffers::GetTranLerpFactor() const noexcept -> int32_t
{
  return m_tranLerpFactor;
}

inline auto ZoomFilterBuffers::TransformBuffers::SetTranLerpFactor(const int32_t val) noexcept
    -> void
{
  m_tranLerpFactor = val;
}

inline auto ZoomFilterBuffers::TransformBuffers::GetSrceDestLerpBufferPoint(
    const size_t buffPos) const noexcept -> Point2dInt
{
  bool isClipped = false;
  return GetSrceDestLerpBufferPoint(buffPos, isClipped);
}

inline auto ZoomFilterBuffers::TransformBuffers::GetSrceDestLerpBufferPoint(
    const size_t buffPos, bool& isClipped) const noexcept -> Point2dInt
{
  const int32_t x =
      GetTranBuffLerpVal(m_tranXSrce[buffPos], m_tranXDest[buffPos], m_tranLerpFactor);
  const int32_t y =
      GetTranBuffLerpVal(m_tranYSrce[buffPos], m_tranYDest[buffPos], m_tranLerpFactor);

  if (x < 0)
  {
    isClipped = true;
    return {0, GetClampedYVal(y)};
  }
  if (x > m_maxTranPointMinus1.x)
  {
    isClipped = true;
    return {m_maxTranPointMinus1.x, GetClampedYVal(y)};
  }

  if (y < 0)
  {
    isClipped = true;
    return {GetClampedXVal(x), 0};
  }
  if (y > m_maxTranPointMinus1.y)
  {
    isClipped = true;
    return {GetClampedXVal(x), m_maxTranPointMinus1.y};
  }

  return {x, y};
}

inline auto ZoomFilterBuffers::TransformBuffers::GetClampedXVal(const int32_t x) const noexcept
    -> int32_t
{
  return std::clamp(x, 0, m_maxTranPointMinus1.x);
}

inline auto ZoomFilterBuffers::TransformBuffers::GetClampedYVal(const int32_t y) const noexcept
    -> int32_t
{
  return std::clamp(y, 0, m_maxTranPointMinus1.y);
}

inline auto ZoomFilterBuffers::TransformBuffers::GetTranBuffLerpVal(const int32_t srceBuffVal,
                                                                    const int32_t destBuffVal,
                                                                    const int32_t t) noexcept
    -> int32_t
{
  // IMPORTANT: Looking at this mathematically I can't see that the '>> DIM_FILTER_COEFFS'
  //            should be there. Surely it should be '/ MAX_TRAN_LERP_VALUE' - but with this,
  //            slight static artifacts appear in the centre of the image. I also tried
  //            '/ (MAX_TRAN_LERP_VALUE - 1)'  which was better with the artifacts in
  //            the centre but may have produced other artifacts.

  return srceBuffVal + ((t * (destBuffVal - srceBuffVal)) >> DIM_FILTER_COEFFS);
}

} // namespace FILTER_FX
} // namespace GOOM
