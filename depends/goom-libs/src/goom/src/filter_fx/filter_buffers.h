#pragma once

#include "goom_plugin_info.h"
#include "goom_types.h"
#include "normalized_coords.h"
#include "point2d.h"
#include "utils/math/misc.h"

#include <cstdint>
#include <memory>
#include <span>

namespace GOOM::FILTER_FX
{

namespace FILTER_BUFFERS
{
// TODO - Is this necessary?
inline constexpr float MIN_SCREEN_COORD_ABS_VAL = 1.0F / 16.0F;
} // namespace FILTER_BUFFERS

template<class FilterStriper>
class ZoomFilterBuffers
{
public:
  enum class TranBuffersState
  {
    START_FRESH_TRAN_BUFFERS,
    RESET_TRAN_BUFFERS,
    TRAN_BUFFERS_READY,
  };

  ZoomFilterBuffers(const PluginInfo& goomInfo,
                    std::unique_ptr<FilterStriper> filterStriper) noexcept;

  auto Start() noexcept -> void;

  [[nodiscard]] auto GetBuffMidpoint() const noexcept -> Point2dInt;
  auto SetBuffMidpoint(const Point2dInt& val) noexcept -> void;

  [[nodiscard]] auto GetFilterViewport() const noexcept -> Viewport;
  auto SetFilterViewport(const Viewport& val) noexcept -> void;

  [[nodiscard]] auto GetTranLerpFactor() const noexcept -> uint32_t;
  auto SetTranLerpFactor(uint32_t val) noexcept -> void;
  static constexpr auto MAX_TRAN_LERP_VALUE = 65536U;

  [[nodiscard]] auto GetTranBuffYLineStart() const noexcept -> uint32_t;

  auto NotifyFilterSettingsHaveChanged() noexcept -> void;
  [[nodiscard]] auto HaveFilterSettingsChanged() const noexcept -> bool;

  [[nodiscard]] auto IsTranBufferFltReady() const noexcept -> bool;
  auto CopyTranBufferFlt(std_spn::span<Point2dFlt>& destBuff) noexcept -> void;

  auto UpdateTranBuffers() noexcept -> void;
  [[nodiscard]] auto GetTranBuffersState() const noexcept -> TranBuffersState;

private:
  Dimensions m_dimensions;
  std::unique_ptr<FilterStriper> m_filterStriper;
  uint32_t m_tranLerpFactor = 0U;

  bool m_filterSettingsHaveChanged    = false;
  TranBuffersState m_tranBuffersState = TranBuffersState::TRAN_BUFFERS_READY;

  auto InitAllTranBuffers() noexcept -> void;
  auto StartFreshTranBuffers() noexcept -> void;
  auto ResetTranBuffers() noexcept -> void;
  auto UpdateNextTranBufferStripe() noexcept -> void;
  auto FillNextTranBuffer() noexcept -> void;
};

template<class FilterStriper>
ZoomFilterBuffers<FilterStriper>::ZoomFilterBuffers(
    const PluginInfo& goomInfo, std::unique_ptr<FilterStriper> filterStriper) noexcept
  : m_dimensions{goomInfo.GetDimensions()}, m_filterStriper{std::move(filterStriper)}
{
}

template<class FilterStriper>
inline auto ZoomFilterBuffers<FilterStriper>::IsTranBufferFltReady() const noexcept -> bool
{
  return m_filterStriper->IsTranBufferFltReady();
}

template<class FilterStriper>
inline auto ZoomFilterBuffers<FilterStriper>::CopyTranBufferFlt(
    std_spn::span<Point2dFlt>& destBuff) noexcept -> void
{
  m_filterStriper->CopyTranBufferFlt(destBuff);
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
  return m_tranLerpFactor;
}

template<class FilterStriper>
inline auto ZoomFilterBuffers<FilterStriper>::SetTranLerpFactor(const uint32_t val) noexcept -> void
{
  m_tranLerpFactor = val;
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
inline auto ZoomFilterBuffers<FilterStriper>::GetTranBuffYLineStart() const noexcept -> uint32_t
{
  return m_filterStriper->GetTranBuffYLineStart();
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

  FillNextTranBuffer();

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
    UpdateNextTranBufferStripe();
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
  m_filterStriper->ResetTranBufferFltIsReady();
  m_tranBuffersState = TranBuffersState::TRAN_BUFFERS_READY;
}

template<class FilterStriper>
inline auto ZoomFilterBuffers<FilterStriper>::ResetTranBuffers() noexcept -> void
{
  m_tranLerpFactor = 0;
  m_filterStriper->ResetStripes();
  m_tranBuffersState = TranBuffersState::START_FRESH_TRAN_BUFFERS;
}

template<class FilterStriper>
inline auto ZoomFilterBuffers<FilterStriper>::UpdateNextTranBufferStripe() noexcept -> void
{
  m_filterStriper->UpdateNextStripe();
  if (0 == m_filterStriper->GetTranBuffYLineStart())
  {
    m_tranBuffersState = TranBuffersState::RESET_TRAN_BUFFERS;
  }
}

template<class FilterStriper>
inline auto ZoomFilterBuffers<FilterStriper>::FillNextTranBuffer() noexcept -> void
{
  m_filterStriper->UpdateAllStripes();
}

} // namespace GOOM::FILTER_FX
