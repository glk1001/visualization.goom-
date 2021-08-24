#ifndef VISUALIZATION_GOOM_FILTER_BUFFERS_H
#define VISUALIZATION_GOOM_FILTER_BUFFERS_H

#include "goom_graphic.h"
#include "v2d.h"

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

class NormalizedCoords;

class ZoomFilterBuffers
{
public:
  static constexpr int32_t DIM_FILTER_COEFFS = 16;
  static constexpr size_t NUM_NEIGHBOR_COEFFS = 4;
  using NeighborhoodPixelArray = std::array<Pixel, NUM_NEIGHBOR_COEFFS>;

  enum class TranBuffersState
  {
    _NULL = -1,
    RESTART_TRAN_BUFFERS,
    RESET_TRAN_BUFFERS,
    TRAN_BUFFERS_READY,
  };

  using ZoomPointFunc = std::function<NormalizedCoords(const NormalizedCoords& normalizedCoords)>;

  ZoomFilterBuffers(UTILS::Parallel& p,
                    const std::shared_ptr<const PluginInfo>& goomInfo,
                    const ZoomPointFunc& zoomPointFunc);

  [[nodiscard]] auto GetZoomPointFunc() const -> ZoomPointFunc;

  [[nodiscard]] auto GetTranLerpFactor() const -> int32_t;
  void SetTranLerpFactor(int32_t val);
  [[nodiscard]] static auto GetMaxTranLerpFactor() -> int32_t;

  [[nodiscard]] auto GetTranBuffYLineStart() const -> uint32_t;

  [[nodiscard]] auto GetBuffMidPoint() const -> V2dInt;
  void SetBuffMidPoint(const V2dInt& val);

  void Start();

  void FilterSettingsChanged();
  void UpdateTranBuffer();
  [[nodiscard]] auto GetTranBufferState() const -> TranBuffersState;
  [[nodiscard]] auto GetZoomBufferTranPoint(size_t buffPos) const -> V2dInt;
  [[nodiscard]] auto IsTranPointClipped(const V2dInt& tranPoint) const -> bool;

  using NeighborhoodCoeffArray = struct
  {
    std::array<uint32_t, NUM_NEIGHBOR_COEFFS> val;
    bool isZero;
  };
  [[nodiscard]] auto GetSourcePointInfo(const V2dInt& tranPoint) const
      -> std::pair<V2dInt, NeighborhoodCoeffArray>;

private:
  const uint32_t m_screenWidth;
  const uint32_t m_screenHeight;

  class FilterCoefficients;
  const std::unique_ptr<const FilterCoefficients> m_precalculatedCoeffs;

  [[nodiscard]] auto GetMaxTranX() const -> uint32_t;
  [[nodiscard]] auto GetMaxTranY() const -> uint32_t;

  [[nodiscard]] static auto NormalizedToTranPoint(const NormalizedCoords& normalizedPoint)
      -> V2dInt;

  UTILS::Parallel& m_parallel;
  const ZoomPointFunc m_getZoomPoint;
  class TransformBuffers;
  std::unique_ptr<TransformBuffers> m_transformBuffers;

  V2dInt m_buffMidPoint{};
  bool m_filterSettingsHaveChanged = false;

  const V2dInt m_maxTranPoint;
  const uint32_t m_tranBuffStripeHeight;
  uint32_t m_tranBuffYLineStart = 0;
  TranBuffersState m_tranBufferState = TranBuffersState::TRAN_BUFFERS_READY;

  std::vector<int32_t> m_firedec{};

  void InitTranBuffers();
  void RestartTranBuffers();
  void ResetTranBuffers();
  void DoNextTranBufferStripe(uint32_t tranBuffStripeHeight);
  void GenerateWaterFxHorizontalBuffer();
  [[nodiscard]] static auto GetTranPoint(const NormalizedCoords& normalized) -> V2dInt;

  // For optimising multiplication, division, and mod by DIM_FILTER_COEFFS.
  static constexpr int32_t DIM_FILTER_COEFFS_DIV_SHIFT = 4;
  static constexpr int32_t DIM_FILTER_COEFFS_MOD_MASK = 0xF;
  static constexpr int32_t MAX_TRAN_LERP_VALUE = 0xFFFF;
  static constexpr float MIN_SCREEN_COORD_ABS_VAL = 1.0F / static_cast<float>(DIM_FILTER_COEFFS);

  [[nodiscard]] static auto TranCoordToCoeffIndex(uint32_t tranCoord) -> uint32_t;
  [[nodiscard]] static auto TranToScreenPoint(const V2dInt& tranPoint) -> V2dInt;
  [[nodiscard]] static auto ScreenToTranPoint(const V2dInt& screenPoint) -> V2dInt;
  [[nodiscard]] static auto ScreenToTranCoord(float screenCoord) -> uint32_t;
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
};

class ZoomFilterBuffers::TransformBuffers
{
public:
  TransformBuffers(uint32_t screenWidth, uint32_t screenHeight) noexcept;

  void SetSrceTranToIdentity();
  void CopyTempTranToDestTran();
  void SaveCurrentDestStateToSrceTran();
  void SetUpNextDestTran();

  void SetTempTransformPoint(uint32_t pos, const V2dInt& transformPoint);

  [[nodiscard]] auto GetTranLerpFactor() const -> int32_t;
  void SetTranLerpFactor(int32_t val);

  [[nodiscard]] auto GetZoomBufferSrceDestLerp(size_t buffPos) const -> V2dInt;

private:
  const uint32_t m_screenWidth;
  const uint32_t m_screenHeight;
  const uint32_t m_bufferSize;
  std::vector<int32_t> m_tranXSrce{};
  std::vector<int32_t> m_tranYSrce{};
  std::vector<int32_t> m_tranXDest{};
  std::vector<int32_t> m_tranYDest{};
  std::vector<int32_t> m_tranXTemp{};
  std::vector<int32_t> m_tranYTemp{};
  int32_t m_tranLerpFactor = 0;

  void CopyAllDestTranToSrceTran();
  void CopyRemainingDestTranToSrceTran();
  [[nodiscard]] static auto GetTranBuffLerpVal(int32_t srceBuffVal, int32_t destBuffVal, int32_t t)
      -> int32_t;
};

inline auto ZoomFilterBuffers::GetBuffMidPoint() const -> V2dInt
{
  return m_buffMidPoint;
}

inline void ZoomFilterBuffers::SetBuffMidPoint(const V2dInt& val)
{
  m_buffMidPoint = val;
}

inline auto ZoomFilterBuffers::GetTranBufferState() const -> TranBuffersState
{
  return m_tranBufferState;
}

inline auto ZoomFilterBuffers::GetTranLerpFactor() const -> int32_t
{
  return m_transformBuffers->GetTranLerpFactor();
}

inline auto ZoomFilterBuffers::TransformBuffers::GetTranLerpFactor() const -> int32_t
{
  return m_tranLerpFactor;
}

inline auto ZoomFilterBuffers::GetMaxTranLerpFactor() -> int32_t
{
  return MAX_TRAN_LERP_VALUE;
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

inline auto ZoomFilterBuffers::IsTranPointClipped(const V2dInt& tranPoint) const -> bool
{
  return (tranPoint.x < 0) || (tranPoint.y < 0) ||
         (static_cast<uint32_t>(tranPoint.x) >= GetMaxTranX()) ||
         (static_cast<uint32_t>(tranPoint.y) >= GetMaxTranY());
}

inline auto ZoomFilterBuffers::GetZoomBufferTranPoint(size_t buffPos) const -> V2dInt
{
  return m_transformBuffers->GetZoomBufferSrceDestLerp(buffPos);
}

inline auto ZoomFilterBuffers::TransformBuffers::GetZoomBufferSrceDestLerp(
    const size_t buffPos) const -> V2dInt
{
  return {GetTranBuffLerpVal(m_tranXSrce[buffPos], m_tranXDest[buffPos], m_tranLerpFactor),
          GetTranBuffLerpVal(m_tranYSrce[buffPos], m_tranYDest[buffPos], m_tranLerpFactor)};
}

inline auto ZoomFilterBuffers::TransformBuffers::GetTranBuffLerpVal(const int32_t srceBuffVal,
                                                                    const int32_t destBuffVal,
                                                                    const int32_t t) -> int32_t
{
  return srceBuffVal + ((t * (destBuffVal - srceBuffVal)) >> DIM_FILTER_COEFFS);
}

inline auto ZoomFilterBuffers::GetZoomPointFunc() const -> ZoomPointFunc
{
  return m_getZoomPoint;
}

} // namespace FILTERS
} // namespace GOOM

#endif
