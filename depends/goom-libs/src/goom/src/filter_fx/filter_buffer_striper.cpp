#include "filter_buffer_striper.h"

//#include "debugging_logger.h"
#include "goom_config.h"
#include "goom_logger.h"
#include "goom_plugin_info.h"
#include "normalized_coords.h"
#include "point2d.h"
#include "utils/math/misc.h"
#include "utils/parallel_utils.h"
#include "utils/t_values.h"

#include <cmath>

namespace GOOM::FILTER_FX
{

using UTILS::IncrementedValue;
using UTILS::Parallel;
using UTILS::TValue;
using UTILS::MATH::FloatsEqual;

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

auto ZoomFilterBufferStriper::InitTranBufferDest(const Dimensions& dimensions,
                                                 std_spn::span<Point2dFlt>& tranBufferFlt) noexcept
    -> void
{
  Expects(dimensions.GetSize() == tranBufferFlt.size());

  IncrementedValue<float> yCoord{NormalizedCoords::MIN_COORD,
                                 NormalizedCoords::MAX_COORD,
                                 TValue::StepType::SINGLE_CYCLE,
                                 dimensions.GetHeight() - 1};
  auto index = size_t{0U};
  for (auto y = 0U; y < dimensions.GetHeight(); ++y)
  {
    IncrementedValue<float> xCoord{NormalizedCoords::MIN_COORD,
                                   NormalizedCoords::MAX_COORD,
                                   TValue::StepType::SINGLE_CYCLE,
                                   dimensions.GetWidth() - 1};
    for (auto x = 0U; x < dimensions.GetWidth(); ++x)
    {
      tranBufferFlt[index] = Point2dFlt{xCoord(), yCoord()};
      xCoord.Increment();
      ++index;
    }
    yCoord.Increment();
  }

  /**
  LogInfo(UTILS::GetGoomLogger(),
          "back().x = {}, max = {}",
          tranBufferFlt.back().x,
          NormalizedCoords::MAX_COORD);
  LogInfo(UTILS::GetGoomLogger(),
          "back().y = {}, max = {}",
          tranBufferFlt.back().y,
          NormalizedCoords::MAX_COORD);
**/

  Ensures(FloatsEqual(tranBufferFlt.front().x, NormalizedCoords::MIN_COORD, 0.0001F));
  Ensures(FloatsEqual(tranBufferFlt.front().y, NormalizedCoords::MIN_COORD, 0.0001F));
  Ensures(FloatsEqual(tranBufferFlt.back().x, NormalizedCoords::MAX_COORD, 0.0001F));
  Ensures(FloatsEqual(tranBufferFlt.back().y, NormalizedCoords::MAX_COORD, 0.0001F));
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
  Expects(m_tranBufferFlt.size() == m_dimensions.GetSize());

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
      m_tranBufferFlt[tranBufferPos] = zoomCoords.GetFltCoords();

      const auto uncenteredZoomCoords = m_normalizedMidpoint + zoomCoords;
      m_tranBuffer[tranBufferPos]     = GetTranPoint(uncenteredZoomCoords);

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
