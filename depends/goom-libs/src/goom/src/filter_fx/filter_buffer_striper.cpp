#include "filter_buffer_striper.h"

#include "goom_config.h"
#include "goom_logger.h"
#include "goom_plugin_info.h"
#include "normalized_coords.h"
#include "point2d.h"
#include "utils/parallel_utils.h"

#include <cmath>

namespace GOOM::FILTER_FX
{

using UTILS::Parallel;

ZoomFilterBufferStriper::ZoomFilterBufferStriper(
    Parallel& parallel,
    const PluginInfo& goomInfo,
    const NormalizedCoordsConverter& normalizedCoordsConverter,
    const ZoomPointFunc& zoomPointFunc) noexcept
  : m_dimensions{goomInfo.GetDimensions()},
    m_normalizedCoordsConverter{&normalizedCoordsConverter},
    m_parallel{&parallel},
    m_getZoomPoint{zoomPointFunc},
    m_tranBuffer(m_dimensions.GetSize())
{
}

auto ZoomFilterBufferStriper::ResetStripes() noexcept -> void
{
  m_tranBuffYLineStart = 0;
}

inline auto ZoomFilterBufferStriper::GetTranPoint(const NormalizedCoords& normalized) const noexcept
    -> Point2dInt
{
  return m_coordTransforms.NormalizedToTranPoint(normalized);
}

/*
 * Makes a stripe of a transform buffer
 *
 * The transform is (in order) :
 * Translation (-data->middleX, -data->middleY)
 * Homothetie (Center : 0,0   Coeff : 2/data->screenWidth)
 */
auto ZoomFilterBufferStriper::DoNextStripe(const uint32_t tranBuffStripeHeight) noexcept -> void
{
  const auto screenWidth                  = m_dimensions.GetWidth();
  const auto screenSpan                   = static_cast<float>(screenWidth - 1);
  const auto sourceCoordsStepSize         = NormalizedCoords::COORD_WIDTH / screenSpan;
  const auto sourceViewportCoordsStepSize = m_filterViewport.GetViewportWidth() / screenSpan;

  const auto doStripeLine =
      [this, &screenWidth, &sourceCoordsStepSize, &sourceViewportCoordsStepSize](const size_t y)
  {
    // Y-position of the first stripe pixel to compute in screen coordinates.
    const auto yScreenCoord = static_cast<uint32_t>(y) + m_tranBuffYLineStart;
    auto tranBufferPos      = yScreenCoord * screenWidth;

    auto centredSourceCoords =
        m_normalizedCoordsConverter->OtherToNormalizedCoords(GetPoint2dInt(0U, yScreenCoord)) -
        m_normalizedMidpoint;
    auto centredSourceViewportCoords = m_filterViewport.GetViewportCoords(centredSourceCoords);

    for (auto x = 0U; x < screenWidth; ++x)
    {
      const auto zoomCoords = m_getZoomPoint(centredSourceCoords, centredSourceViewportCoords);
      const auto uncenteredZoomCoords = m_normalizedMidpoint + zoomCoords;

      m_tranBuffer[tranBufferPos] = GetTranPoint(uncenteredZoomCoords);

      centredSourceCoords.IncX(sourceCoordsStepSize);
      centredSourceViewportCoords.IncX(sourceViewportCoordsStepSize);
      ++tranBufferPos;
    }
  };

  // Where (vertically) to stop generating the buffer stripe.
  const auto tranBuffYLineEnd =
      std::min(m_dimensions.GetHeight(), m_tranBuffYLineStart + tranBuffStripeHeight);
  const auto numStripes = static_cast<size_t>(tranBuffYLineEnd - m_tranBuffYLineStart);

  m_parallel->ForLoop(numStripes, doStripeLine);

  m_tranBuffYLineStart += tranBuffStripeHeight;
  if (tranBuffYLineEnd >= m_dimensions.GetHeight())
  {
    m_tranBuffYLineStart = 0;
  }
}

} // namespace GOOM::FILTER_FX
