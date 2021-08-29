#include "filter_buffers.h"

#include "../stats/filter_stats.h"
#include "filter_normalized_coords.h"
#include "goom_graphic.h"
#include "goom_plugin_info.h"
#include "goomutils/goomrand.h"
#include "goomutils/logging_control.h"
//#undef NO_LOGGING
#include "goomutils/logging.h"
#include "goomutils/parallel_utils.h"
#include "v2d.h"

#include <array>
#undef NDEBUG
#include <cassert>
#include <cmath>
#include <cstdint>
#include <memory>
#include <vector>

#if __cplusplus <= 201402L
namespace GOOM
{
namespace FILTERS
{
#else
namespace GOOM::FILTERS
{
#endif

using UTILS::GetNRand;
using UTILS::GetRandInRange;
using UTILS::Logging;
using UTILS::Parallel;

ZoomFilterBuffers::ZoomFilterBuffers(Parallel& p,
                                     const std::shared_ptr<const PluginInfo>& goomInfo,
                                     const ZoomPointFunc& zoomPointFunc,
                                     FilterStats& stats)
  : m_screenWidth{goomInfo->GetScreenInfo().width},
    m_screenHeight{goomInfo->GetScreenInfo().height},
    m_stats{stats},
    m_precalculatedCoeffs{std::make_unique<FilterCoefficients>()},
    m_parallel{p},
    m_getZoomPoint{zoomPointFunc},
    m_transformBuffers{std::make_unique<TransformBuffers>(m_screenWidth, m_screenHeight)},
    m_maxTranPoint{CoordTransforms::ScreenToTranPoint(
        {static_cast<int32_t>(m_screenWidth - 1), static_cast<int32_t>(m_screenHeight - 1)})},
    m_tranBuffStripeHeight{m_screenHeight / DIM_FILTER_COEFFS},
    m_firedec(m_screenHeight)
{
  assert(DIM_FILTER_COEFFS == static_cast<int32_t>(std::lround(
                                  std::pow(2, CoordTransforms::DIM_FILTER_COEFFS_DIV_SHIFT))));
  assert(CoordTransforms::MAX_TRAN_LERP_VALUE ==
         static_cast<int32_t>(std::lround(std::pow(2, DIM_FILTER_COEFFS))) - 1);

  NormalizedCoords::SetScreenDimensions(m_screenWidth, m_screenHeight, MIN_SCREEN_COORD_ABS_VAL);
}

void ZoomFilterBuffers::Start()
{
  InitAllTranBuffers();
}

auto ZoomFilterBuffers::GetSourcePointInfo(const V2dInt& tranPoint) const
    -> std::pair<V2dInt, NeighborhoodCoeffArray>
{
  const V2dInt srcePoint = CoordTransforms::TranToScreenPoint(tranPoint);
  const size_t xIndex = CoordTransforms::TranCoordToCoeffIndex(static_cast<uint32_t>(tranPoint.x));
  const size_t yIndex = CoordTransforms::TranCoordToCoeffIndex(static_cast<uint32_t>(tranPoint.y));
  return std::make_pair(srcePoint, m_precalculatedCoeffs->GetCoeffs()[xIndex][yIndex]);
}

auto ZoomFilterBuffers::HaveFilterSettingsChanged() const -> bool
{
  return m_filterSettingsHaveChanged;
}

void ZoomFilterBuffers::NotifyFilterSettingsHaveChanged()
{
  m_filterSettingsHaveChanged = true;
}

void ZoomFilterBuffers::InitAllTranBuffers()
{
  GenerateWaterFxHorizontalBuffer();

  FillTempTranBuffers();

  m_transformBuffers->SetSrceTranToIdentity();
  m_transformBuffers->CopyTempTranToDestTran();

  m_tranBuffYLineStart = 0;
  m_tranBuffersState = TranBuffersState::START_FRESH_TRAN_BUFFERS;
}

void ZoomFilterBuffers::UpdateTranBuffers()
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
    // Create a new destination stripe of 'm_tranBuffStripeHeight' height starting
    // at 'm_tranBuffYLineStart'.
    DoNextTempTranBuffersStripe(m_tranBuffStripeHeight);
  }
}

// generation du buffer de transform
void ZoomFilterBuffers::ResetTranBuffers()
{
  m_stats.DoResetTranBuffers();

  m_transformBuffers->CopyDestTranToSrceTran();
  m_transformBuffers->SetUpNextDestTran();

  m_transformBuffers->SetTranLerpFactor(0);
  m_tranBuffYLineStart = 0;
  m_tranBuffersState = TranBuffersState::START_FRESH_TRAN_BUFFERS;
}

void ZoomFilterBuffers::StartFreshTranBuffers()
{
  if (!m_filterSettingsHaveChanged)
  {
    return;
  }

  m_stats.DoStartFreshTranBuffers();

  m_filterSettingsHaveChanged = false;
  m_tranBuffYLineStart = 0;
  m_tranBuffersState = TranBuffersState::TRAN_BUFFERS_READY;
}

inline void ZoomFilterBuffers::FillTempTranBuffers()
{
  DoNextTempTranBuffersStripe(m_screenHeight);
}

/*
 * Makes a stripe of a transform buffer
 *
 * The transform is (in order) :
 * Translation (-data->middleX, -data->middleY)
 * Homothetie (Center : 0,0   Coeff : 2/data->screenWidth)
 */
void ZoomFilterBuffers::DoNextTempTranBuffersStripe(const uint32_t tranBuffStripeHeight)
{
  assert(m_tranBuffersState == TranBuffersState::TRAN_BUFFERS_READY);

  const NormalizedCoords normalizedMidPt{m_buffMidPoint}; //TODO optimize

  const auto doStripeLine = [&](const uint32_t y) {
    // Position of the pixel to compute in screen coordinates
    const uint32_t yOffset = y + m_tranBuffYLineStart;
    const uint32_t tranPosStart = yOffset * m_screenWidth;

    NormalizedCoords normalizedCentredPoint =
        NormalizedCoords{V2dInt{0, static_cast<int32_t>(yOffset)}} - normalizedMidPt;

    for (uint32_t x = 0; x < m_screenWidth; ++x)
    {
      const NormalizedCoords normalizedZoomPoint = m_getZoomPoint(normalizedCentredPoint);
      const NormalizedCoords uncenteredZoomPoint = normalizedMidPt + normalizedZoomPoint;

      m_transformBuffers->SetTempBuffersTransformPoint(tranPosStart + x,
                                                       GetTranPoint(uncenteredZoomPoint));

      normalizedCentredPoint.IncX();
    }
  };

  // Where (vertically) to stop generating the buffer stripe
  const uint32_t tranBuffYLineEnd =
      std::min(m_screenHeight, m_tranBuffYLineStart + tranBuffStripeHeight);

  m_parallel.ForLoop(tranBuffYLineEnd - m_tranBuffYLineStart, doStripeLine);

  m_tranBuffYLineStart += tranBuffStripeHeight;

  if (tranBuffYLineEnd >= m_screenHeight)
  {
    m_tranBuffersState = TranBuffersState::RESET_TRAN_BUFFERS;
    m_tranBuffYLineStart = 0;
  }
}

inline auto ZoomFilterBuffers::GetTranPoint(const NormalizedCoords& normalized) -> V2dInt
{
  return CoordTransforms::NormalizedToTranPoint(normalized);

  /**
  int32_t tranX = NormalizedToTranPoint(xNormalised);
  if (tranX < 0)
  {
    tranX = -1;
  }
  else if (tranX >= static_cast<int32_t>(m_maxTranX))
  {
    //    tranX = static_cast<int32_t>(m_maxTranX - 1);
    tranX = static_cast<int32_t>(m_maxTranX);
  }

  int32_t tranY = NormalizedToTranPoint(yNormalised);
  if (tranY < 0)
  {
    tranY = -1;
  }
  else if (tranY >= static_cast<int32_t>(m_maxTranY))
  {
    //    tranY = static_cast<int32_t>(m_maxTranY - 1);
    tranY = static_cast<int32_t>(m_maxTranY);
  }

  return std::make_tuple(tranX, tranY);
   **/
}

void ZoomFilterBuffers::GenerateWaterFxHorizontalBuffer()
{
  int32_t decc = GetRandInRange(-4, +4);
  int32_t spdc = GetRandInRange(-4, +4);
  int32_t accel = GetRandInRange(-4, +4);

  for (size_t loopv = m_screenHeight; loopv != 0;)
  {
    --loopv;
    m_firedec[loopv] = decc;
    decc += spdc / 10;
    spdc += GetRandInRange(-2, +3);

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
      spdc = (spdc - static_cast<int32_t>(GetNRand(3))) + (accel / 10);
    }
    if (spdc < -30)
    {
      spdc = spdc + static_cast<int32_t>(GetNRand(3)) + (accel / 10);
    }

    if ((decc > 8) && (spdc > 1))
    {
      spdc -= GetRandInRange(-2, +1);
    }
    if ((decc < -8) && (spdc < -1))
    {
      spdc += static_cast<int32_t>(GetNRand(3)) + 2;
    }
    if ((decc > 8) || (decc < -8))
    {
      decc = (decc * 8) / 9;
    }

    accel += GetRandInRange(-1, +2);
    if (accel > 20)
    {
      accel -= 2;
    }
    if (accel < -20)
    {
      accel += 2;
    }
  }
}

ZoomFilterBuffers::TransformBuffers::TransformBuffers(const uint32_t screenWidth,
                                                      const uint32_t screenHeight) noexcept
  : m_screenWidth{screenWidth},
    m_screenHeight{screenHeight},
    m_bufferSize{m_screenWidth * m_screenHeight},
    m_tranXSrce(m_bufferSize),
    m_tranYSrce(m_bufferSize),
    m_tranXDest(m_bufferSize),
    m_tranYDest(m_bufferSize),
    m_tranXTemp(m_bufferSize),
    m_tranYTemp(m_bufferSize)
{
}

void ZoomFilterBuffers::TransformBuffers::SetSrceTranToIdentity()
{
  size_t i = 0;
  for (int32_t y = 0; y < static_cast<int32_t>(m_screenHeight); ++y)
  {
    for (int32_t x = 0; x < static_cast<int32_t>(m_screenWidth); ++x)
    {
      const V2dInt tranPoint = CoordTransforms::ScreenToTranPoint({x, y});
      m_tranXSrce[i] = tranPoint.x;
      m_tranYSrce[i] = tranPoint.y;
      ++i;
    }
  }
}

inline void ZoomFilterBuffers::TransformBuffers::CopyTempTranToDestTran()
{
  std::copy(m_tranXTemp.begin(), m_tranXTemp.end(), m_tranXDest.begin());
  std::copy(m_tranYTemp.begin(), m_tranYTemp.end(), m_tranYDest.begin());
}

inline void ZoomFilterBuffers::TransformBuffers::CopyAllDestTranToSrceTran()
{
  std::copy(m_tranXDest.begin(), m_tranXDest.end(), m_tranXSrce.begin());
  std::copy(m_tranYDest.begin(), m_tranYDest.end(), m_tranYSrce.begin());
}

void ZoomFilterBuffers::TransformBuffers::CopyUnlerpedDestTranToSrceTran()
{
  for (size_t i = 0; i < m_bufferSize; ++i)
  {
    const V2dInt tranPoint = GetSrceDestLerpBufferPoint(i);
    m_tranXSrce[i] = tranPoint.x;
    m_tranYSrce[i] = tranPoint.y;
  }
}

void ZoomFilterBuffers::TransformBuffers::CopyDestTranToSrceTran()
{
  // sauvegarde de l'etat actuel dans la nouvelle source
  // Save the current state in the source buffs.
  if (0 == GetTranLerpFactor())
  {
    // Nothing to do: tran srce == tran dest.
  }
  else if (GetTranLerpFactor() == CoordTransforms::MAX_TRAN_LERP_VALUE)
  {
    CopyAllDestTranToSrceTran();
  }
  else
  {
    CopyUnlerpedDestTranToSrceTran();
  }
}

inline void ZoomFilterBuffers::TransformBuffers::SetUpNextDestTran()
{
  std::swap(m_tranXDest, m_tranXTemp);
  std::swap(m_tranYDest, m_tranYTemp);
}

inline void ZoomFilterBuffers::TransformBuffers::SetTempBuffersTransformPoint(
    const uint32_t pos, const V2dInt& transformPoint)
{
  m_tranXTemp[pos] = transformPoint.x;
  m_tranYTemp[pos] = transformPoint.y;
}

// TODO Old Clang and MSVC won't allow the following '= default'
//ZoomFilterBuffers::FilterCoefficients::FilterCoefficients() noexcept = default;
ZoomFilterBuffers::FilterCoefficients::FilterCoefficients() noexcept
{
}

inline auto ZoomFilterBuffers::FilterCoefficients::GetCoeffs() const -> const FilterCoeff2dArray&
{
  return m_precalculatedCoeffs;
}

auto ZoomFilterBuffers::FilterCoefficients::GetPrecalculatedCoefficients() -> FilterCoeff2dArray
{
  FilterCoeff2dArray precalculatedCoeffs{};

  for (uint32_t coeffH = 0; coeffH < DIM_FILTER_COEFFS; ++coeffH)
  {
    for (uint32_t coeffV = 0; coeffV < DIM_FILTER_COEFFS; ++coeffV)
    {
      const uint32_t diffCoeffH = DIM_FILTER_COEFFS - coeffH;
      const uint32_t diffCoeffV = DIM_FILTER_COEFFS - coeffV;

      if ((0 == coeffH) && (0 == coeffV))
      {
        precalculatedCoeffs[coeffH][coeffV] = {{channel_limits<uint32_t>::max(), 0U, 0U, 0U},
                                               false};
      }
      else
      {
        uint32_t i1 = diffCoeffH * diffCoeffV;
        uint32_t i2 = coeffH * diffCoeffV;
        uint32_t i3 = diffCoeffH * coeffV;
        uint32_t i4 = coeffH * coeffV;

        // TODO: faire mieux...
        if (i1)
        {
          --i1;
        }
        if (i2)
        {
          --i2;
        }
        if (i3)
        {
          --i3;
        }
        if (i4)
        {
          --i4;
        }

        /**
        if (ProbabilityOfMInN(1, 100))
        {
          i1 += GetRandInRange(0U, 10U);
        }
        if (ProbabilityOfMInN(1, 100))
        {
          i2 += GetRandInRange(0U, 10U);
        }
        if (ProbabilityOfMInN(1, 100))
        {
          i3 += GetRandInRange(0U, 10U);
        }
        if (ProbabilityOfMInN(1, 100))
        {
          i4 += GetRandInRange(0U, 10U);
        }
        i1 = 16;
        i2 =  20;
        i3 = 20;
        i4 = 16;
**/
        precalculatedCoeffs[coeffH][coeffV] = {{i1, i2, i3, i4},
                                               (0 == i1) && (0 == i2) && (0 == i3) && (0 == i4)};
      }
    }
  }

  return precalculatedCoeffs;
}

#if __cplusplus <= 201402L
} // namespace FILTERS
} // namespace GOOM
#else
} // namespace GOOM::DRAW
#endif
