#pragma once

#include "filter_buffer_stripes.h"
#include "filter_utils/zoom_coord_transforms.h"
#include "filter_utils/zoom_filter_coefficients.h"
#include "filter_utils/zoom_transform_buffers.h"
#include "goom_types.h"
#include "normalized_coords.h"
#include "point2d.h"

#include <cstdint>
#include <functional>
#include <vector>

namespace GOOM
{
class PluginInfo;

namespace UTILS
{
class Parallel;
}
} // namespace GOOM

namespace GOOM::FILTER_FX
{

class ZoomFilterBuffers
{
public:
  static constexpr float MIN_SCREEN_COORD_ABS_VAL =
      1.0F / static_cast<float>(FILTER_UTILS::ZOOM_FILTER_COEFFS::DIM_FILTER_COEFFS);

  enum class TranBuffersState
  {
    START_FRESH_TRAN_BUFFERS,
    RESET_TRAN_BUFFERS,
    TRAN_BUFFERS_READY,
  };

  using ZoomPointFunc =
      std::function<NormalizedCoords(const NormalizedCoords& normalizedCoords,
                                     const NormalizedCoords& normalizedFilterViewportCoords)>;

  ZoomFilterBuffers(UTILS::Parallel& parallel,
                    const PluginInfo& goomInfo,
                    const NormalizedCoordsConverter& normalizedCoordsConverter,
                    const ZoomPointFunc& zoomPointFunc) noexcept;

  [[nodiscard]] auto GetBuffMidpoint() const noexcept -> Point2dInt;
  auto SetBuffMidpoint(const Point2dInt& val) noexcept -> void;

  [[nodiscard]] auto GetFilterViewport() const noexcept -> Viewport;
  auto SetFilterViewport(const Viewport& val) noexcept -> void;

  [[nodiscard]] auto GetTranLerpFactor() const noexcept -> uint32_t;
  auto SetTranLerpFactor(uint32_t val) noexcept -> void;
  [[nodiscard]] static auto GetMaxTranLerpFactor() noexcept -> uint32_t;

  auto Start() noexcept -> void;

  [[nodiscard]] auto GetTranBuffYLineStart() const noexcept -> uint32_t;

  auto NotifyFilterSettingsHaveChanged() noexcept -> void;
  [[nodiscard]] auto HaveFilterSettingsChanged() const noexcept -> bool;

  auto UpdateTranBuffers() noexcept -> void;
  [[nodiscard]] auto GetTranBuffersState() const noexcept -> TranBuffersState;

  struct SourcePointInfo
  {
    Point2dInt screenPoint;
    const FILTER_UTILS::ZOOM_FILTER_COEFFS::NeighborhoodCoeffArray* coeffs;
    bool isClipped;
  };
  [[nodiscard]] auto GetSourcePointInfo(size_t buffPos) const noexcept -> SourcePointInfo;

  [[nodiscard]] auto GetZoomBufferTranPoint(size_t buffPos, bool& isClipped) const noexcept
      -> Point2dInt;

  [[nodiscard]] auto GetTransformBuffers() const noexcept
      -> const FILTER_UTILS::ZoomTransformBuffers&;

private:
  const Dimensions m_dimensions;
  ZoomFilterBufferStripes m_filterBufferStripes;

  using ZoomCoordTransforms       = FILTER_UTILS::ZoomCoordTransforms;
  const Point2dInt m_maxTranPoint = ZoomCoordTransforms::ScreenToTranPoint(
      {m_dimensions.GetIntWidth() - 1, m_dimensions.GetIntHeight() - 1});
  FILTER_UTILS::ZoomTransformBuffers m_transformBuffers{m_dimensions, m_maxTranPoint};

  bool m_filterSettingsHaveChanged    = false;
  TranBuffersState m_tranBuffersState = TranBuffersState::TRAN_BUFFERS_READY;

  [[nodiscard]] auto GetMaxTranX() const noexcept -> uint32_t;
  [[nodiscard]] auto GetMaxTranY() const noexcept -> uint32_t;

  auto InitAllTranBuffers() noexcept -> void;
  auto StartFreshTranBuffers() noexcept -> void;
  auto ResetTranBuffers() noexcept -> void;
  auto UpdateNextTempTranBufferStripe() noexcept -> void;
  auto FillTempTranBuffer() noexcept -> void;
  auto GenerateWaterFxHorizontalBuffer() noexcept -> void;

  std::vector<int32_t> m_firedec;
};

inline auto ZoomFilterBuffers::GetBuffMidpoint() const noexcept -> Point2dInt
{
  return m_filterBufferStripes.GetBuffMidpoint();
}

inline auto ZoomFilterBuffers::SetBuffMidpoint(const Point2dInt& val) noexcept -> void
{
  m_filterBufferStripes.SetBuffMidpoint(val);
}

inline auto ZoomFilterBuffers::GetFilterViewport() const noexcept -> Viewport
{
  return m_filterBufferStripes.GetFilterViewport();
}

inline auto ZoomFilterBuffers::SetFilterViewport(const Viewport& val) noexcept -> void
{
  m_filterBufferStripes.SetFilterViewport(val);
}

inline auto ZoomFilterBuffers::GetTranBuffersState() const noexcept -> TranBuffersState
{
  return m_tranBuffersState;
}

inline auto ZoomFilterBuffers::GetTranLerpFactor() const noexcept -> uint32_t
{
  return m_transformBuffers.GetTranLerpFactor();
}

inline auto ZoomFilterBuffers::GetMaxTranLerpFactor() noexcept -> uint32_t
{
  return FILTER_UTILS::ZoomTransformBuffers::MAX_TRAN_LERP_VALUE;
}

inline auto ZoomFilterBuffers::SetTranLerpFactor(const uint32_t val) noexcept -> void
{
  m_transformBuffers.SetTranLerpFactor(val);
}

inline auto ZoomFilterBuffers::HaveFilterSettingsChanged() const noexcept -> bool
{
  return m_filterSettingsHaveChanged;
}

inline auto ZoomFilterBuffers::NotifyFilterSettingsHaveChanged() noexcept -> void
{
  m_filterSettingsHaveChanged = true;
}

inline auto ZoomFilterBuffers::GetZoomBufferTranPoint(const size_t buffPos,
                                                      bool& isClipped) const noexcept -> Point2dInt
{
  return m_transformBuffers.GetSrceDestLerpBufferPoint(buffPos, isClipped);
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
  return m_filterBufferStripes.GetTranBuffYLineStart();
}

inline auto ZoomFilterBuffers::GetTransformBuffers() const noexcept
    -> const FILTER_UTILS::ZoomTransformBuffers&
{
  return m_transformBuffers;
}

inline auto ZoomFilterBuffers::GetSourcePointInfo(const size_t buffPos) const noexcept
    -> SourcePointInfo
{
  using FILTER_UTILS::ZOOM_FILTER_COEFFS::PRECALCULATED_COEFF_PTRS;

  auto isClipped       = false;
  const auto tranPoint = GetZoomBufferTranPoint(buffPos, isClipped);

  const auto srceScreenPoint  = ZoomCoordTransforms::TranToScreenPoint(tranPoint);
  const auto [xIndex, yIndex] = ZoomCoordTransforms::TranCoordToZoomCoeffIndexes(tranPoint);

  return SourcePointInfo{srceScreenPoint, PRECALCULATED_COEFF_PTRS[xIndex][yIndex], isClipped};
}

} // namespace GOOM::FILTER_FX
