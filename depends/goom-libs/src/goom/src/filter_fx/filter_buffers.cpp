//#undef NO_LOGGING

#include "filter_buffers.h"

#include "goom_config.h"
#include "goom_logger.h"
#include "goom_plugin_info.h"
#include "normalized_coords.h"
#include "utils/parallel_utils.h"

namespace GOOM::FILTER_FX
{

using UTILS::Parallel;

ZoomFilterBuffers::ZoomFilterBuffers(Parallel& parallel,
                                     const PluginInfo& goomInfo,
                                     const NormalizedCoordsConverter& normalizedCoordsConverter,
                                     const ZoomPointFunc& zoomPointFunc) noexcept
  : m_dimensions{goomInfo.GetScreenDimensions()},
    m_filterBufferStripes{parallel, goomInfo, normalizedCoordsConverter, zoomPointFunc},
    m_firedec(m_dimensions.GetHeight())
{
}

auto ZoomFilterBuffers::Start() noexcept -> void
{
  InitAllTranBuffers();
}

inline auto ZoomFilterBuffers::InitAllTranBuffers() noexcept -> void
{
  m_tranBuffersState = TranBuffersState::TRAN_BUFFERS_READY;
  m_filterBufferStripes.ResetStripes();

  GenerateWaterFxHorizontalBuffer();

  FillTempTranBuffer();

  m_transformBuffers.SetSrceTranToIdentity();
  m_transformBuffers.SwapDestTran(m_filterBufferStripes.GetTranBuffer());

  m_filterBufferStripes.ResetStripes();
  m_tranBuffersState = TranBuffersState::START_FRESH_TRAN_BUFFERS;
}

auto ZoomFilterBuffers::UpdateTranBuffers() noexcept -> void
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

inline auto ZoomFilterBuffers::StartFreshTranBuffers() noexcept -> void
{
  if (not m_filterSettingsHaveChanged)
  {
    return;
  }

  m_filterSettingsHaveChanged = false;
  m_filterBufferStripes.ResetStripes();
  m_tranBuffersState = TranBuffersState::TRAN_BUFFERS_READY;
}

inline auto ZoomFilterBuffers::ResetTranBuffers() noexcept -> void
{
  m_transformBuffers.CopyDestTranToSrceTran();
  m_transformBuffers.SwapDestTran(m_filterBufferStripes.GetTranBuffer());

  m_transformBuffers.SetTranLerpFactor(0);
  m_filterBufferStripes.ResetStripes();
  m_tranBuffersState = TranBuffersState::START_FRESH_TRAN_BUFFERS;
}

inline auto ZoomFilterBuffers::UpdateNextTempTranBufferStripe() noexcept -> void
{
  m_filterBufferStripes.UpdateNextStripe();
  if (m_filterBufferStripes.GetTranBuffYLineStart() == 0)
  {
    m_tranBuffersState = TranBuffersState::RESET_TRAN_BUFFERS;
  }
}

inline auto ZoomFilterBuffers::FillTempTranBuffer() noexcept -> void
{
  m_filterBufferStripes.UpdateAllStripes();
}

auto ZoomFilterBuffers::GenerateWaterFxHorizontalBuffer() noexcept -> void
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
