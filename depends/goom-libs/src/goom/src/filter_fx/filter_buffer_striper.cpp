#undef NO_LOGGING
#include "filter_buffer_striper.h"

#include "debugging_logger.h"
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

  static constexpr auto MIN_COORD   = -2.0F;
  static constexpr auto MAX_COORD   = +2.0F;
  static constexpr auto COORD_WIDTH = MAX_COORD - MIN_COORD;
  const float xRatioScreenToNormalizedCoord =
      COORD_WIDTH / static_cast<float>(dimensions.GetWidth());
  const float yRatioScreenToNormalizedCoord =
      COORD_WIDTH / static_cast<float>(dimensions.GetHeight());

  const auto getNormalizedCoords = [&xRatioScreenToNormalizedCoord, &yRatioScreenToNormalizedCoord](
                                       const float x, const float y) noexcept -> Point2dFlt
  {
    return {MIN_COORD + (xRatioScreenToNormalizedCoord * x),
            MIN_COORD + (yRatioScreenToNormalizedCoord * y)};
  };

  for (auto y = 0U; y < dimensions.GetHeight(); ++y)
  {
    const auto yIndex = static_cast<size_t>(y) * static_cast<size_t>(dimensions.GetWidth());
    for (auto x = 0U; x < dimensions.GetWidth(); ++x)
    {
      const auto index = yIndex + static_cast<size_t>(x);

      const auto identityXY =
          getNormalizedCoords(0.5F + static_cast<float>(x), 0.5F + static_cast<float>(y));

      tranBufferFlt[index] = identityXY;
    }
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

  //  Ensures(FloatsEqual(tranBufferFlt.front().x, NormalizedCoords::MIN_COORD, 0.0001F));
  //  Ensures(FloatsEqual(tranBufferFlt.front().y, NormalizedCoords::MIN_COORD, 0.0001F));
  //  Ensures(FloatsEqual(tranBufferFlt.back().x, NormalizedCoords::MAX_COORD, 0.0001F));
  //  Ensures(FloatsEqual(tranBufferFlt.back().y, NormalizedCoords::MAX_COORD, 0.0001F));
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
  LogInfo(GoomLogger(), "Starting next stripe.");
  Expects(m_tranBufferFlt.size() == m_dimensions.GetSize());


  //  static constexpr auto X_OFFSET = 0.01F;
  static auto s_numGetFrameDataRequests = 0U;
  //  const bool updateOffset        = (s_numGetFrameDataRequests % 2000000000) == 0;
  ++s_numGetFrameDataRequests;


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
      //const auto zoomCoords = m_getZoomPoint(centredSourceCoords, centredSourceViewportCoords);
      const auto zoomCoords           = centredSourceCoords;
      const auto uncenteredZoomCoords = m_normalizedMidpoint + zoomCoords;

      //m_tranBufferFlt[tranBufferPos]  = zoomCoords.GetFltCoords();
      m_tranBufferFlt[tranBufferPos] = uncenteredZoomCoords.GetFltCoords();

      m_tranBuffer[tranBufferPos] = GetTranPoint(uncenteredZoomCoords);

      // NOT SETTING m_tranBufferFlt AT ALL HAS RIGHT DRIFT
      //const auto zoomInFactor = 1.0F - 0.001F;
      //m_tranBufferFlt[tranBufferPos] = (m_normalizedMidpoint + zoomInFactor*centredSourceCoords).GetFltCoords();

      // THIS SORT OF WORKS
      //      if (updateOffset)
      //      {
      //        auto destXY = m_tranBufferFlt[tranBufferPos];
      //        destXY      = {destXY.x + X_OFFSET, destXY.y};
      //        if (destXY.x > NormalizedCoords::MAX_COORD)
      //        {
      //          destXY.x = NormalizedCoords::MIN_COORD;
      //        }
      //        m_tranBufferFlt[tranBufferPos] = destXY;
      //      }

      if ((s_numGetFrameDataRequests % 10) == 0)
      {
        LogInfo(GoomLogger(),
                "m_tranBuffer[{}]    = {}, {}",
                tranBufferPos,
                m_tranBuffer[tranBufferPos].x,
                m_tranBuffer[tranBufferPos].y);
        LogInfo(GoomLogger(),
                "m_tranBufferFlt[{}] = {}, {}",
                tranBufferPos,
                m_tranBufferFlt[tranBufferPos].x,
                m_tranBufferFlt[tranBufferPos].y);
      }

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
