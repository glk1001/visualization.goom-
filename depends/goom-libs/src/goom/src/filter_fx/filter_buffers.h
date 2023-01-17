#pragma once

#include "filter_utils/zoom_coord_transforms.h"
#include "filter_utils/zoom_filter_coefficients.h"
#include "filter_utils/zoom_transform_buffers.h"
#include "goom_plugin_info.h"
#include "goom_types.h"
#include "point2d.h"

#include <cstdint>
#include <functional>
#include <memory>
#include <vector>

namespace GOOM::FILTER_FX
{

namespace FILTER_BUFFERS
{
inline constexpr float MIN_SCREEN_COORD_ABS_VAL =
    1.0F / static_cast<float>(FILTER_UTILS::ZOOM_FILTER_COEFFS::DIM_FILTER_COEFFS);

struct SourcePointInfo
{
  Point2dInt screenPoint;
  const FILTER_UTILS::ZOOM_FILTER_COEFFS::NeighborhoodCoeffArray* coeffs;
  bool isClipped;
};
} // namespace FILTER_BUFFERS

template<class FilterStriper>
class ZoomFilterBuffers
{
  using ZoomTransformBuffers = FILTER_UTILS::ZoomTransformBuffers;
  using ZoomCoordTransforms  = FILTER_UTILS::ZoomCoordTransforms;
  using SourcePointInfo      = FILTER_BUFFERS::SourcePointInfo;

public:
  enum class TranBuffersState
  {
    START_FRESH_TRAN_BUFFERS,
    RESET_TRAN_BUFFERS,
    TRAN_BUFFERS_READY,
  };

  ZoomFilterBuffers(const PluginInfo& goomInfo,
                    std::unique_ptr<FilterStriper> filterStriper) noexcept;

  [[nodiscard]] auto GetBuffMidpoint() const noexcept -> Point2dInt;
  auto SetBuffMidpoint(const Point2dInt& val) noexcept -> void;

  [[nodiscard]] auto GetFilterViewport() const noexcept -> Viewport;
  auto SetFilterViewport(const Viewport& val) noexcept -> void;

  [[nodiscard]] auto GetTranLerpFactor() const noexcept -> uint32_t;
  auto SetTranLerpFactor(uint32_t val) noexcept -> void;
  [[nodiscard]] static auto GetMaxTranLerpFactor() noexcept -> uint32_t;

  auto Start() noexcept -> void;

  [[nodiscard]] auto GetTranBuffYLineStart() const noexcept -> uint32_t;
  [[nodiscard]] auto GetTransformBuffers() const noexcept -> const ZoomTransformBuffers&;

  auto NotifyFilterSettingsHaveChanged() noexcept -> void;
  [[nodiscard]] auto HaveFilterSettingsChanged() const noexcept -> bool;

  auto UpdateTranBuffers() noexcept -> void;
  [[nodiscard]] auto GetTranBuffersState() const noexcept -> TranBuffersState;
  [[nodiscard]] auto GetSourcePointInfo(size_t buffPos) const noexcept -> SourcePointInfo;
  [[nodiscard]] auto GetZoomBufferTranPoint(size_t buffPos, bool& isClipped) const noexcept
      -> Point2dInt;

private:
  const Dimensions m_dimensions;
  std::unique_ptr<FilterStriper> m_filterStriper;

  const Point2dInt m_maxTranPoint = ZoomCoordTransforms::ScreenToTranPoint(
      {m_dimensions.GetIntWidth() - 1, m_dimensions.GetIntHeight() - 1});
  ZoomTransformBuffers m_transformBuffers{m_dimensions, m_maxTranPoint};

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

template<class FilterStriper>
ZoomFilterBuffers<FilterStriper>::ZoomFilterBuffers(
    const PluginInfo& goomInfo, std::unique_ptr<FilterStriper> filterStriper) noexcept
  : m_dimensions{goomInfo.GetScreenDimensions()},
    m_filterStriper{std::move(filterStriper)},
    m_firedec(m_dimensions.GetHeight())
{
}

template<class FilterStriper>
inline auto ZoomFilterBuffers<FilterStriper>::GetBuffMidpoint() const noexcept -> Point2dInt
{
  return m_filterStriper->GetBuffMidpoint();
}

template<class FilterStriper>
inline auto ZoomFilterBuffers<FilterStriper>::SetBuffMidpoint(const Point2dInt& val) noexcept
    -> void
{
  m_filterStriper->SetBuffMidpoint(val);
}

template<class FilterStriper>
inline auto ZoomFilterBuffers<FilterStriper>::GetFilterViewport() const noexcept -> Viewport
{
  return m_filterStriper->GetFilterViewport();
}

template<class FilterStriper>
inline auto ZoomFilterBuffers<FilterStriper>::SetFilterViewport(const Viewport& val) noexcept
    -> void
{
  m_filterStriper->SetFilterViewport(val);
}

template<class FilterStriper>
inline auto ZoomFilterBuffers<FilterStriper>::GetTranBuffersState() const noexcept
    -> TranBuffersState
{
  return m_tranBuffersState;
}

template<class FilterStriper>
inline auto ZoomFilterBuffers<FilterStriper>::GetTranLerpFactor() const noexcept -> uint32_t
{
  return m_transformBuffers.GetTranLerpFactor();
}

template<class FilterStriper>
inline auto ZoomFilterBuffers<FilterStriper>::GetMaxTranLerpFactor() noexcept -> uint32_t
{
  return ZoomTransformBuffers::MAX_TRAN_LERP_VALUE;
}

template<class FilterStriper>
inline auto ZoomFilterBuffers<FilterStriper>::SetTranLerpFactor(const uint32_t val) noexcept -> void
{
  m_transformBuffers.SetTranLerpFactor(val);
}

template<class FilterStriper>
inline auto ZoomFilterBuffers<FilterStriper>::HaveFilterSettingsChanged() const noexcept -> bool
{
  return m_filterSettingsHaveChanged;
}

template<class FilterStriper>
inline auto ZoomFilterBuffers<FilterStriper>::NotifyFilterSettingsHaveChanged() noexcept -> void
{
  m_filterSettingsHaveChanged = true;
}

template<class FilterStriper>
inline auto ZoomFilterBuffers<FilterStriper>::GetZoomBufferTranPoint(const size_t buffPos,
                                                                     bool& isClipped) const noexcept
    -> Point2dInt
{
  return m_transformBuffers.GetSrceDestLerpBufferPoint(buffPos, isClipped);
}

template<class FilterStriper>
inline auto ZoomFilterBuffers<FilterStriper>::GetMaxTranX() const noexcept -> uint32_t
{
  return static_cast<uint32_t>(m_maxTranPoint.x);
}

template<class FilterStriper>
inline auto ZoomFilterBuffers<FilterStriper>::GetMaxTranY() const noexcept -> uint32_t
{
  return static_cast<uint32_t>(m_maxTranPoint.y);
}

template<class FilterStriper>
inline auto ZoomFilterBuffers<FilterStriper>::GetTranBuffYLineStart() const noexcept -> uint32_t
{
  return m_filterStriper->GetTranBuffYLineStart();
}

template<class FilterStriper>
inline auto ZoomFilterBuffers<FilterStriper>::GetTransformBuffers() const noexcept
    -> const ZoomTransformBuffers&
{
  return m_transformBuffers;
}

template<class FilterStriper>
inline auto ZoomFilterBuffers<FilterStriper>::GetSourcePointInfo(
    const size_t buffPos) const noexcept -> SourcePointInfo
{
  using FILTER_UTILS::ZOOM_FILTER_COEFFS::PRECALCULATED_COEFF_PTRS;

  auto isClipped       = false;
  const auto tranPoint = GetZoomBufferTranPoint(buffPos, isClipped);

  const auto srceScreenPoint  = ZoomCoordTransforms::TranToScreenPoint(tranPoint);
  const auto [xIndex, yIndex] = ZoomCoordTransforms::TranCoordToZoomCoeffIndexes(tranPoint);

  return {srceScreenPoint, PRECALCULATED_COEFF_PTRS[xIndex][yIndex], isClipped};
}

template<class FilterStriper>
inline auto ZoomFilterBuffers<FilterStriper>::Start() noexcept -> void
{
  InitAllTranBuffers();
}

template<class FilterStriper>
inline auto ZoomFilterBuffers<FilterStriper>::InitAllTranBuffers() noexcept -> void
{
  m_tranBuffersState = TranBuffersState::TRAN_BUFFERS_READY;
  m_filterStriper->ResetStripes();

  GenerateWaterFxHorizontalBuffer();

  FillTempTranBuffer();

  m_transformBuffers.SetSrceTranToIdentity();
  m_transformBuffers.SwapDestTran(m_filterStriper->GetTranBuffer());

  m_filterStriper->ResetStripes();
  m_tranBuffersState = TranBuffersState::START_FRESH_TRAN_BUFFERS;
}

template<class FilterStriper>
inline auto ZoomFilterBuffers<FilterStriper>::UpdateTranBuffers() noexcept -> void
{
  if (m_tranBuffersState == TranBuffersState::RESET_TRAN_BUFFERS)
  {
    ResetTranBuffers();
  }
  else if (m_tranBuffersState == TranBuffersState::START_FRESH_TRAN_BUFFERS)
  {
    StartFreshTranBuffers();
  }
  else
  {
    UpdateNextTempTranBufferStripe();
  }
}

template<class FilterStriper>
inline auto ZoomFilterBuffers<FilterStriper>::StartFreshTranBuffers() noexcept -> void
{
  if (not m_filterSettingsHaveChanged)
  {
    return;
  }

  m_filterSettingsHaveChanged = false;
  m_filterStriper->ResetStripes();
  m_tranBuffersState = TranBuffersState::TRAN_BUFFERS_READY;
}

template<class FilterStriper>
inline auto ZoomFilterBuffers<FilterStriper>::ResetTranBuffers() noexcept -> void
{
  m_transformBuffers.CopyDestTranToSrceTran();
  m_transformBuffers.SwapDestTran(m_filterStriper->GetTranBuffer());

  m_transformBuffers.SetTranLerpFactor(0);
  m_filterStriper->ResetStripes();
  m_tranBuffersState = TranBuffersState::START_FRESH_TRAN_BUFFERS;
}

template<class FilterStriper>
inline auto ZoomFilterBuffers<FilterStriper>::UpdateNextTempTranBufferStripe() noexcept -> void
{
  m_filterStriper->UpdateNextStripe();
  if (m_filterStriper->GetTranBuffYLineStart() == 0)
  {
    m_tranBuffersState = TranBuffersState::RESET_TRAN_BUFFERS;
  }
}

template<class FilterStriper>
inline auto ZoomFilterBuffers<FilterStriper>::FillTempTranBuffer() noexcept -> void
{
  m_filterStriper->UpdateAllStripes();
}

template<class FilterStriper>
auto ZoomFilterBuffers<FilterStriper>::GenerateWaterFxHorizontalBuffer() noexcept -> void
{
  /*****************
  int32_t decc = m_goomRand.GetRandInRange(-4, +4);
  int32_t spdc = m_goomRand.GetRandInRange(-4, +4);
  int32_t accel = m_goomRand.GetRandInRange(-4, +4);

  for (size_t loopv = m_screenHeight; loopv != 0;)
  {
    --loopv;
    m_firedec[loopv] = decc;
    decc += spdc / 10;
    spdc += m_goomRand.GetRandInRange(-2, +3);

    if (decc > 4)
    {
      spdc -= 1;
    }
    if (decc < -4)
    {
      spdc += 1;
    }

    if (spdc > 30)
    {
      spdc = (spdc - static_cast<int32_t>(m_goomRand.GetNRand(3))) + (accel / 10);
    }
    if (spdc < -30)
    {
      spdc = spdc + static_cast<int32_t>(m_goomRand.GetNRand(3)) + (accel / 10);
    }

    if ((decc > 8) && (spdc > 1))
    {
      spdc -= m_goomRand.GetRandInRange(-2, +1);
    }
    if ((decc < -8) && (spdc < -1))
    {
      spdc += static_cast<int32_t>(m_goomRand.GetNRand(3)) + 2;
    }
    if ((decc > 8) || (decc < -8))
    {
      decc = (decc * 8) / 9;
    }

    accel += m_goomRand.GetRandInRange(-1, +2);
    if (accel > 20)
    {
      accel -= 2;
    }
    if (accel < -20)
    {
      accel += 2;
    }
  }
  *************************/
}

} // namespace GOOM::FILTER_FX
