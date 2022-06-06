#include "filter_buffers.h"

//#undef NO_LOGGING

#include "goom/logging.h"
#include "goom_config.h"
#include "goom_graphic.h"
#include "goom_plugin_info.h"
#include "normalized_coords.h"
#include "point2d.h"
#include "utils/parallel_utils.h"

#include <array>
#include <cmath>
#include <cstdint>
#include <memory>
#include <numeric>
#include <vector>

namespace GOOM::FILTER_FX
{

using UTILS::Logging;
using UTILS::Parallel;

ZoomFilterBuffers::ZoomFilterBuffers(Parallel& parallel,
                                     const PluginInfo& goomInfo,
                                     const NormalizedCoordsConverter& normalizedCoordsConverter,
                                     const ZoomPointFunc& zoomPointFunc) noexcept
  : m_screenWidth{goomInfo.GetScreenInfo().width},
    m_screenHeight{goomInfo.GetScreenInfo().height},
    m_normalizedCoordsConverter{normalizedCoordsConverter},
    m_parallel{parallel},
    m_getZoomPoint{zoomPointFunc},
    m_maxTranPoint{CoordTransforms::ScreenToTranPoint(
        {static_cast<int32_t>(m_screenWidth - 1), static_cast<int32_t>(m_screenHeight - 1)})},
    m_firedec(m_screenHeight)
{
  Expects(DIM_FILTER_COEFFS == static_cast<int32_t>(std::lround(
                                   std::pow(2, CoordTransforms::DIM_FILTER_COEFFS_DIV_SHIFT))));
  Expects(CoordTransforms::MAX_TRAN_LERP_VALUE ==
          static_cast<int32_t>(std::lround(std::pow(2, DIM_FILTER_COEFFS))) - 1);
}

ZoomFilterBuffers::~ZoomFilterBuffers() noexcept = default;

auto ZoomFilterBuffers::Start() noexcept -> void
{
  InitAllTranBuffers();
}

auto ZoomFilterBuffers::GetSourcePointInfo(const size_t buffPos) const noexcept -> SourcePointInfo
{
  bool isClipped = false;
  const Point2dInt tranPoint = GetZoomBufferTranPoint(buffPos, isClipped);

  const Point2dInt srceScreenPoint = CoordTransforms::TranToScreenPoint(tranPoint);
  const size_t xIndex = CoordTransforms::TranCoordToCoeffIndex(static_cast<uint32_t>(tranPoint.x));
  const size_t yIndex = CoordTransforms::TranCoordToCoeffIndex(static_cast<uint32_t>(tranPoint.y));

  return SourcePointInfo{srceScreenPoint, m_precalculatedCoeffs->GetCoeffs()[xIndex][yIndex],
                         isClipped};
}

inline auto ZoomFilterBuffers::GetZoomBufferTranPoint(const size_t buffPos,
                                                      bool& isClipped) const noexcept -> Point2dInt
{
  return m_transformBuffers->GetSrceDestLerpBufferPoint(buffPos, isClipped);
}

auto ZoomFilterBuffers::HaveFilterSettingsChanged() const noexcept -> bool
{
  return m_filterSettingsHaveChanged;
}

auto ZoomFilterBuffers::NotifyFilterSettingsHaveChanged() noexcept -> void
{
  m_filterSettingsHaveChanged = true;
}

auto ZoomFilterBuffers::InitAllTranBuffers() noexcept -> void
{
  m_tranBuffersState = TranBuffersState::TRAN_BUFFERS_READY;

  GenerateWaterFxHorizontalBuffer();

  FillTempTranBuffers();

  m_transformBuffers->SetSrceTranToIdentity();
  m_transformBuffers->CopyTempTranToDestTran();

  m_tranBuffYLineStart = 0;
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
    // Create a new destination stripe of 'm_tranBuffStripeHeight' height starting
    // at 'm_tranBuffYLineStart'.
    DoNextTempTranBuffersStripe(m_tranBuffStripeHeight);
  }
}

// generation du buffer de transform
auto ZoomFilterBuffers::ResetTranBuffers() noexcept -> void
{
  m_transformBuffers->CopyDestTranToSrceTran();
  m_transformBuffers->SetUpNextDestTran();

  m_transformBuffers->SetTranLerpFactor(0);
  m_tranBuffYLineStart = 0;
  m_tranBuffersState = TranBuffersState::START_FRESH_TRAN_BUFFERS;
}

auto ZoomFilterBuffers::StartFreshTranBuffers() noexcept -> void
{
  if (!m_filterSettingsHaveChanged)
  {
    return;
  }

  m_filterSettingsHaveChanged = false;
  m_tranBuffYLineStart = 0;
  m_tranBuffersState = TranBuffersState::TRAN_BUFFERS_READY;
}

inline auto ZoomFilterBuffers::FillTempTranBuffers() noexcept -> void
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
auto ZoomFilterBuffers::DoNextTempTranBuffersStripe(const uint32_t tranBuffStripeHeight) noexcept
    -> void
{
  Expects(m_tranBuffersState == TranBuffersState::TRAN_BUFFERS_READY);

  const auto doStripeLine = [this](const size_t y)
  {
    // Position of the pixel to compute in screen coordinates
    const uint32_t yOffset = static_cast<uint32_t>(y) + m_tranBuffYLineStart;
    const uint32_t tranPosStart = yOffset * m_screenWidth;

    NormalizedCoords normalizedCentredPoint = m_normalizedCoordsConverter.ScreenToNormalizedCoords(
                                                  Point2dInt{0, static_cast<int32_t>(yOffset)}) -
                                              m_normalizedMidPt;

    for (uint32_t x = 0; x < m_screenWidth; ++x)
    {
      const NormalizedCoords normalizedZoomPoint = m_getZoomPoint(normalizedCentredPoint);
      const NormalizedCoords uncenteredZoomPoint = m_normalizedMidPt + normalizedZoomPoint;

      m_transformBuffers->SetTempBuffersTransformPoint(tranPosStart + x,
                                                       GetTranPoint(uncenteredZoomPoint));

      m_normalizedCoordsConverter.IncX(normalizedCentredPoint);
    }
  };

  // Where (vertically) to stop generating the buffer stripe
  const uint32_t tranBuffYLineEnd =
      std::min(m_screenHeight, m_tranBuffYLineStart + tranBuffStripeHeight);

  const auto numStripes = static_cast<size_t>(tranBuffYLineEnd - m_tranBuffYLineStart);

  m_parallel.ForLoop(numStripes, doStripeLine);

  m_tranBuffYLineStart += tranBuffStripeHeight;

  if (tranBuffYLineEnd >= m_screenHeight)
  {
    m_tranBuffersState = TranBuffersState::RESET_TRAN_BUFFERS;
    m_tranBuffYLineStart = 0;
  }
}

inline auto ZoomFilterBuffers::GetTranPoint(const NormalizedCoords& normalized) const noexcept
    -> Point2dInt
{
  return m_coordTransforms->NormalizedToTranPoint(normalized);

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

ZoomFilterBuffers::TransformBuffers::TransformBuffers(const uint32_t screenWidth,
                                                      const uint32_t screenHeight,
                                                      const Point2dInt& maxTranPoint) noexcept
  : m_screenWidth{screenWidth},
    m_screenHeight{screenHeight},
    m_bufferSize{m_screenWidth * m_screenHeight},
    m_maxTranPointMinus1{maxTranPoint - Vec2dInt{1, 1}},
    m_tranXSrce(m_bufferSize),
    m_tranYSrce(m_bufferSize),
    m_tranXDest(m_bufferSize),
    m_tranYDest(m_bufferSize),
    m_tranXTemp(m_bufferSize),
    m_tranYTemp(m_bufferSize)
{
}

auto ZoomFilterBuffers::TransformBuffers::SetSrceTranToIdentity() noexcept -> void
{
  size_t i = 0;
  for (int32_t y = 0; y < static_cast<int32_t>(m_screenHeight); ++y)
  {
    for (int32_t x = 0; x < static_cast<int32_t>(m_screenWidth); ++x)
    {
      const Point2dInt tranPoint = CoordTransforms::ScreenToTranPoint({x, y});
      m_tranXSrce[i] = tranPoint.x;
      m_tranYSrce[i] = tranPoint.y;
      ++i;
    }
  }
}

inline auto ZoomFilterBuffers::TransformBuffers::CopyTempTranToDestTran() noexcept -> void
{
  std::copy(m_tranXTemp.begin(), m_tranXTemp.end(), m_tranXDest.begin());
  std::copy(m_tranYTemp.begin(), m_tranYTemp.end(), m_tranYDest.begin());
}

inline auto ZoomFilterBuffers::TransformBuffers::CopyAllDestTranToSrceTran() noexcept -> void
{
  std::copy(m_tranXDest.begin(), m_tranXDest.end(), m_tranXSrce.begin());
  std::copy(m_tranYDest.begin(), m_tranYDest.end(), m_tranYSrce.begin());
}

auto ZoomFilterBuffers::TransformBuffers::CopyUnlerpedDestTranToSrceTran() noexcept -> void
{
  for (size_t i = 0; i < m_bufferSize; ++i)
  {
    const Point2dInt tranPoint = GetSrceDestLerpBufferPoint(i);
    m_tranXSrce[i] = tranPoint.x;
    m_tranYSrce[i] = tranPoint.y;
  }
}

auto ZoomFilterBuffers::TransformBuffers::CopyDestTranToSrceTran() noexcept -> void
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

inline auto ZoomFilterBuffers::TransformBuffers::SetUpNextDestTran() noexcept -> void
{
  std::swap(m_tranXDest, m_tranXTemp);
  std::swap(m_tranYDest, m_tranYTemp);
}

inline auto ZoomFilterBuffers::TransformBuffers::SetTempBuffersTransformPoint(
    const uint32_t pos, const Point2dInt& transformPoint) noexcept -> void
{
  m_tranXTemp[pos] = transformPoint.x;
  m_tranYTemp[pos] = transformPoint.y;
}

inline auto ZoomFilterBuffers::FilterCoefficients::GetCoeffs() const noexcept
    -> const FilterCoeff2dArray&
{
  return m_precalculatedCoeffs;
}

auto ZoomFilterBuffers::FilterCoefficients::GetPrecalculatedCoefficients() noexcept
    -> FilterCoeff2dArray
{
  FilterCoeff2dArray precalculatedCoeffs{};

  for (uint32_t coeffH = 0; coeffH < DIM_FILTER_COEFFS; ++coeffH)
  {
    for (uint32_t coeffV = 0; coeffV < DIM_FILTER_COEFFS; ++coeffV)
    {
      precalculatedCoeffs[coeffH][coeffV] = GetNeighborhoodCoeffArray(coeffH, coeffV);
    }
  }

  return precalculatedCoeffs;
}

auto ZoomFilterBuffers::FilterCoefficients::GetNeighborhoodCoeffArray(
    const uint32_t coeffH, const uint32_t coeffV) noexcept -> NeighborhoodCoeffArray
{
  const uint32_t diffCoeffH = DIM_FILTER_COEFFS - coeffH;
  const uint32_t diffCoeffV = DIM_FILTER_COEFFS - coeffV;

  // clang-format off
  std::array<uint32_t, NUM_NEIGHBOR_COEFFS> coeffs = {
      diffCoeffH * diffCoeffV,
      coeffH * diffCoeffV,
      diffCoeffH * coeffV,
      coeffH * coeffV
  };
  // clang-format on

  // We want to decrement just one coefficient so that the sum of
  // coefficients equals 255. We'll choose the max coefficient.
  const uint32_t maxCoeff = *std::max_element(cbegin(coeffs), cend(coeffs));
  bool allZero = false;
  if (0 == maxCoeff)
  {
    allZero = true;
  }
  else
  {
    for (auto& coeff : coeffs)
    {
      if (maxCoeff == coeff)
      {
        --coeff;
        break;
      }
    }
  }

  LogInfo("{:2}, {:2}:  {:3}, {:3}, {:3}, {:3} - sum: {:3}", coeffH, coeffV, coeffs[0], coeffs[1],
          coeffs[2], coeffs[3], std::accumulate(cbegin(coeffs), cend(coeffs), 0U));
  Ensures(channel_limits<uint32_t>::max() == std::accumulate(cbegin(coeffs), cend(coeffs), 0U));

  return {coeffs, allZero};
}

} // namespace GOOM::FILTER_FX
