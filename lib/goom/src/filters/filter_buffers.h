#ifndef VISUALIZATION_GOOM_LIB_GOOM_FILTERS_FILTER_BUFFERS_H
#define VISUALIZATION_GOOM_LIB_GOOM_FILTERS_FILTER_BUFFERS_H

#include "goom_graphic.h"
#include "normalized_coords.h"
#include "v2d.h"

#include <array>
#include <cmath>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace GOOM
{

class PluginInfo;
class PixelBuffer;

namespace UTILS
{
class Parallel;
} // namespace UTILS

namespace FILTERS
{

class ZoomFilterBuffers
{
public:
  static constexpr int32_t DIM_FILTER_COEFFS = 16;
  static constexpr size_t NUM_NEIGHBOR_COEFFS = 4;
  using NeighborhoodPixelArray = std::array<Pixel, NUM_NEIGHBOR_COEFFS>;
  class CoordTransforms;

  enum class TranBuffersState
  {
    START_FRESH_TRAN_BUFFERS,
    RESET_TRAN_BUFFERS,
    TRAN_BUFFERS_READY,
  };

  using ZoomPointFunc = std::function<NormalizedCoords(const NormalizedCoords& normalizedCoords)>;

  ZoomFilterBuffers(UTILS::Parallel& p,
                    const PluginInfo& goomInfo,
                    const ZoomPointFunc& zoomPointFunc);

  [[nodiscard]] auto GetTranLerpFactor() const -> int32_t;
  void SetTranLerpFactor(int32_t val);
  [[nodiscard]] static auto GetMaxTranLerpFactor() -> int32_t;

  [[nodiscard]] auto GetTranBuffYLineStart() const -> uint32_t;

  [[nodiscard]] auto GetBuffMidPoint() const -> V2dInt;
  void SetBuffMidPoint(const V2dInt& val);

  void Start();

  void NotifyFilterSettingsHaveChanged();
  auto HaveFilterSettingsChanged() const -> bool;

  void UpdateTranBuffers();
  [[nodiscard]] auto GetTranBuffersState() const -> TranBuffersState;

  struct NeighborhoodCoeffArray
  {
    std::array<uint32_t, NUM_NEIGHBOR_COEFFS> val;
    bool isZero;
  };
  struct SourcePointInfo
  {
    V2dInt screenPoint;
    NeighborhoodCoeffArray coeffs;
    bool isClipped;
  };
  [[nodiscard]] auto GetSourcePointInfo(size_t buffPos) const -> SourcePointInfo;

private:
  const uint32_t m_screenWidth;
  const uint32_t m_screenHeight;

  class FilterCoefficients;
  const std::unique_ptr<const FilterCoefficients> m_precalculatedCoeffs;

  [[nodiscard]] auto GetMaxTranX() const -> uint32_t;
  [[nodiscard]] auto GetMaxTranY() const -> uint32_t;

  UTILS::Parallel& m_parallel;
  const ZoomPointFunc m_getZoomPoint;
  const V2dInt m_maxTranPoint;
  const uint32_t m_tranBuffStripeHeight;
  class TransformBuffers;
  std::unique_ptr<TransformBuffers> m_transformBuffers;

  V2dInt m_buffMidPoint{};
  bool m_filterSettingsHaveChanged = false;

  uint32_t m_tranBuffYLineStart = 0;
  TranBuffersState m_tranBuffersState = TranBuffersState::TRAN_BUFFERS_READY;

  std::vector<int32_t> m_firedec{};

  static constexpr float MIN_SCREEN_COORD_ABS_VAL = 1.0F / static_cast<float>(DIM_FILTER_COEFFS);

  void InitAllTranBuffers();
  void StartFreshTranBuffers();
  void ResetTranBuffers();
  void FillTempTranBuffers();
  void DoNextTempTranBuffersStripe(uint32_t tranBuffStripeHeight);
  void GenerateWaterFxHorizontalBuffer();
  [[nodiscard]] auto GetZoomBufferTranPoint(size_t buffPos, bool& isClipped) const -> V2dInt;
  [[nodiscard]] static auto GetTranPoint(const NormalizedCoords& normalized) -> V2dInt;
};

class ZoomFilterBuffers::CoordTransforms
{
public:
  // Use these consts for optimising multiplication, division, and mod, by DIM_FILTER_COEFFS.
  static constexpr int32_t MAX_TRAN_LERP_VALUE = 0xFFFF;
  static constexpr int32_t DIM_FILTER_COEFFS_DIV_SHIFT = 4;
  static constexpr int32_t DIM_FILTER_COEFFS_MOD_MASK = 0xF;

  [[nodiscard]] static auto TranCoordToCoeffIndex(uint32_t tranCoord) -> uint32_t;
  [[nodiscard]] static auto TranToScreenPoint(const V2dInt& tranPoint) -> V2dInt;
  [[nodiscard]] static auto ScreenToTranPoint(const V2dInt& screenPoint) -> V2dInt;
  [[nodiscard]] static auto ScreenToTranCoord(float screenCoord) -> uint32_t;

  [[nodiscard]] static auto NormalizedToTranPoint(const NormalizedCoords& normalizedPoint)
      -> V2dInt;
};

class ZoomFilterBuffers::FilterCoefficients
{
public:
  FilterCoefficients() noexcept;

  using FilterCoeff2dArray =
      std::array<std::array<NeighborhoodCoeffArray, DIM_FILTER_COEFFS>, DIM_FILTER_COEFFS>;
  [[nodiscard]] auto GetCoeffs() const -> const FilterCoeff2dArray&;

private:
  // modif d'optim by Jeko : precalcul des 4 coeffs resultant des 2 pos
  const FilterCoeff2dArray m_precalculatedCoeffs{GetPrecalculatedCoefficients()};
  [[nodiscard]] static auto GetPrecalculatedCoefficients() -> FilterCoeff2dArray;
  [[nodiscard]] static auto GetNeighborhoodCoeffArray(uint32_t coeffH, uint32_t coeffV)
      -> NeighborhoodCoeffArray;
};

class ZoomFilterBuffers::TransformBuffers
{
public:
  TransformBuffers(uint32_t screenWidth,
                   uint32_t screenHeight,
                   const V2dInt& maxTranPoint) noexcept;

  void SetSrceTranToIdentity();
  void CopyTempTranToDestTran();
  void CopyDestTranToSrceTran();
  void SetUpNextDestTran();

  void SetTempBuffersTransformPoint(uint32_t pos, const V2dInt& transformPoint);

  [[nodiscard]] auto GetTranLerpFactor() const -> int32_t;
  void SetTranLerpFactor(int32_t val);

  [[nodiscard]] auto GetSrceDestLerpBufferPoint(const size_t buffPos, bool& isClipped) const
      -> V2dInt;

private:
  const uint32_t m_screenWidth;
  const uint32_t m_screenHeight;
  const uint32_t m_bufferSize;
  const V2dInt m_maxTranPointMinus1;
  std::vector<int32_t> m_tranXSrce{};
  std::vector<int32_t> m_tranYSrce{};
  std::vector<int32_t> m_tranXDest{};
  std::vector<int32_t> m_tranYDest{};
  std::vector<int32_t> m_tranXTemp{};
  std::vector<int32_t> m_tranYTemp{};
  int32_t m_tranLerpFactor = 0;

  void CopyAllDestTranToSrceTran();
  void CopyUnlerpedDestTranToSrceTran();
  [[nodiscard]] auto GetSrceDestLerpBufferPoint(const size_t buffPos) const -> V2dInt;
  [[nodiscard]] static auto GetTranBuffLerpVal(int32_t srceBuffVal, int32_t destBuffVal, int32_t t)
      -> int32_t;
  [[nodiscard]] static auto GetClippedBufferValue(const int32_t unClippedValue,
                                                  const int32_t maxValue,
                                                  bool& isClipped) -> int32_t;
};

inline auto ZoomFilterBuffers::CoordTransforms::TranCoordToCoeffIndex(const uint32_t tranCoord)
    -> uint32_t
{
  return tranCoord & DIM_FILTER_COEFFS_MOD_MASK;
}

inline auto ZoomFilterBuffers::CoordTransforms::TranToScreenPoint(const V2dInt& tranPoint) -> V2dInt
{
  return {tranPoint.x >> DIM_FILTER_COEFFS_DIV_SHIFT, tranPoint.y >> DIM_FILTER_COEFFS_DIV_SHIFT};
}

inline auto ZoomFilterBuffers::CoordTransforms::ScreenToTranPoint(const V2dInt& screenPoint)
    -> V2dInt
{
  return {screenPoint.x << DIM_FILTER_COEFFS_DIV_SHIFT,
          screenPoint.y << DIM_FILTER_COEFFS_DIV_SHIFT};
}

inline auto ZoomFilterBuffers::CoordTransforms::ScreenToTranCoord(const float screenCoord)
    -> uint32_t
{
  // IMPORTANT: Without 'lround' a faint cross artifact appears in the centre of the screen.
  return static_cast<uint32_t>(std::lround(screenCoord * static_cast<float>(DIM_FILTER_COEFFS)));
}

inline auto ZoomFilterBuffers::CoordTransforms::NormalizedToTranPoint(
    const NormalizedCoords& normalizedPoint) -> V2dInt
{
  const V2dFlt screenCoords = normalizedPoint.GetScreenCoordsFlt();

  // IMPORTANT: Without 'lround' a faint cross artifact appears in the centre of the screen.
  return {static_cast<int32_t>(std::lround(ScreenToTranCoord(screenCoords.x))),
          static_cast<int32_t>(std::lround(ScreenToTranCoord(screenCoords.y)))};
}

inline auto ZoomFilterBuffers::GetBuffMidPoint() const -> V2dInt
{
  return m_buffMidPoint;
}

inline void ZoomFilterBuffers::SetBuffMidPoint(const V2dInt& val)
{
  m_buffMidPoint = val;
}

inline auto ZoomFilterBuffers::GetTranBuffersState() const -> TranBuffersState
{
  return m_tranBuffersState;
}

inline auto ZoomFilterBuffers::GetTranLerpFactor() const -> int32_t
{
  return m_transformBuffers->GetTranLerpFactor();
}

inline auto ZoomFilterBuffers::GetMaxTranLerpFactor() -> int32_t
{
  return CoordTransforms::MAX_TRAN_LERP_VALUE;
}

inline void ZoomFilterBuffers::SetTranLerpFactor(const int32_t val)
{
  m_transformBuffers->SetTranLerpFactor(val);
}

inline auto ZoomFilterBuffers::GetMaxTranX() const -> uint32_t
{
  return static_cast<uint32_t>(m_maxTranPoint.x);
}

inline auto ZoomFilterBuffers::GetMaxTranY() const -> uint32_t
{
  return static_cast<uint32_t>(m_maxTranPoint.y);
}

inline auto ZoomFilterBuffers::GetTranBuffYLineStart() const -> uint32_t
{
  return m_tranBuffYLineStart;
}

inline auto ZoomFilterBuffers::TransformBuffers::GetTranLerpFactor() const -> int32_t
{
  return m_tranLerpFactor;
}

inline void ZoomFilterBuffers::TransformBuffers::SetTranLerpFactor(const int32_t val)
{
  m_tranLerpFactor = val;
}

inline auto ZoomFilterBuffers::TransformBuffers::GetSrceDestLerpBufferPoint(
    const size_t buffPos) const -> V2dInt
{
  bool isClipped = false;
  return GetSrceDestLerpBufferPoint(buffPos, isClipped);
}

inline auto ZoomFilterBuffers::TransformBuffers::GetSrceDestLerpBufferPoint(const size_t buffPos,
                                                                            bool& isClipped) const
    -> V2dInt
{
  const int32_t x = GetClippedBufferValue(
      GetTranBuffLerpVal(m_tranXSrce[buffPos], m_tranXDest[buffPos], m_tranLerpFactor),
      m_maxTranPointMinus1.x, isClipped);
  const int32_t y = GetClippedBufferValue(
      GetTranBuffLerpVal(m_tranYSrce[buffPos], m_tranYDest[buffPos], m_tranLerpFactor),
      m_maxTranPointMinus1.y, isClipped);

  return {x, y};
}

inline auto ZoomFilterBuffers::TransformBuffers::GetClippedBufferValue(const int32_t unClippedValue,
                                                                       const int32_t maxValue,
                                                                       bool& isClipped) -> int32_t
{
  if (unClippedValue < 0)
  {
    isClipped = true;
    return 0;
  }
  if (unClippedValue > maxValue)
  {
    isClipped = true;
    return maxValue;
  }

  return unClippedValue;
}

inline auto ZoomFilterBuffers::TransformBuffers::GetTranBuffLerpVal(const int32_t srceBuffVal,
                                                                    const int32_t destBuffVal,
                                                                    const int32_t t) -> int32_t
{
  // IMPORTANT: Looking at this mathematically I can't see that the '>> DIM_FILTER_COEFFS'
  //            should be there. Surely it should be '/ MAX_TRAN_LERP_VALUE' - but with this,
  //            slight static artifacts appear in the centre of the image. I also tried
  //            '/ (MAX_TRAN_LERP_VALUE - 1)'  which was better with the artifacts in
  //            the centre but may have produced other artifacts.

  return srceBuffVal + ((t * (destBuffVal - srceBuffVal)) >> DIM_FILTER_COEFFS);
}

} // namespace FILTERS
} // namespace GOOM

#endif //VISUALIZATION_GOOM_LIB_GOOM_FILTERS_FILTER_BUFFERS_H
