#pragma once

#include "filter_utils/zoom_coord_transforms.h"
#include "filter_utils/zoom_filter_coefficients.h"
#include "filter_utils/zoom_transform_buffers.h"
#include "goom_graphic.h"
#include "goom_types.h"
#include "normalized_coords.h"
#include "point2d.h"

#include <cmath>
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

class ZoomFilterBufferStriper
{
public:
  using ZoomPointFunc =
      std::function<NormalizedCoords(const NormalizedCoords& normalizedCoords,
                                     const NormalizedCoords& normalizedFilterViewportCoords)>;

  ZoomFilterBufferStriper(UTILS::Parallel& parallel,
                          const PluginInfo& goomInfo,
                          const NormalizedCoordsConverter& normalizedCoordsConverter,
                          const ZoomPointFunc& zoomPointFunc) noexcept;

  [[nodiscard]] auto GetTranBuffYLineStart() const noexcept -> uint32_t;

  [[nodiscard]] auto GetBuffMidpoint() const noexcept -> Point2dInt;
  auto SetBuffMidpoint(const Point2dInt& val) noexcept -> void;

  [[nodiscard]] auto GetFilterViewport() const noexcept -> Viewport;
  auto SetFilterViewport(const Viewport& val) noexcept -> void;

  auto ResetStripes() noexcept -> void;

  auto UpdateAllStripes() noexcept -> void;
  auto UpdateNextStripe() noexcept -> void;

  [[nodiscard]] auto GetTranBuffer() noexcept -> std::vector<Point2dInt>&;

private:
  Dimensions m_dimensions;
  const NormalizedCoordsConverter* m_normalizedCoordsConverter;
  FILTER_UTILS::ZoomCoordTransforms m_coordTransforms{m_dimensions};

  UTILS::Parallel* m_parallel;
  ZoomPointFunc m_getZoomPoint;
  Point2dInt m_buffMidpoint             = {0, 0};
  NormalizedCoords m_normalizedMidpoint = {0.0F, 0.0F};
  Viewport m_filterViewport             = Viewport{};

  // 'NUM_STRIPE_GROUPS' controls how many updates before all stripes, and therefore,
  // all of the tran buffer, is filled. We use stripes to spread the buffer update load
  // over a number of updates. Too few and performance suffers periodically for a
  // number of updates; too many, and performance suffers overall.
  static constexpr auto NUM_STRIPE_GROUPS = 16U;
  uint32_t m_tranBuffYLineStart           = 0;
  uint32_t m_tranBuffStripeHeight         = m_dimensions.GetHeight() / NUM_STRIPE_GROUPS;
  std::vector<Point2dInt> m_tranBuffer;

  auto DoNextStripe(uint32_t tranBuffStripeHeight) noexcept -> void;
  [[nodiscard]] auto GetTranPoint(const NormalizedCoords& normalized) const noexcept -> Point2dInt;
};

inline auto ZoomFilterBufferStriper::GetTranBuffYLineStart() const noexcept -> uint32_t
{
  return m_tranBuffYLineStart;
}

inline auto ZoomFilterBufferStriper::GetTranBuffer() noexcept -> std::vector<Point2dInt>&
{
  return m_tranBuffer;
}

inline auto ZoomFilterBufferStriper::GetBuffMidpoint() const noexcept -> Point2dInt
{
  return m_buffMidpoint;
}

inline auto ZoomFilterBufferStriper::SetBuffMidpoint(const Point2dInt& val) noexcept -> void
{
  m_buffMidpoint       = val;
  m_normalizedMidpoint = m_normalizedCoordsConverter->OtherToNormalizedCoords(m_buffMidpoint);
}

inline auto ZoomFilterBufferStriper::GetFilterViewport() const noexcept -> Viewport
{
  return m_filterViewport;
}

inline auto ZoomFilterBufferStriper::SetFilterViewport(const Viewport& val) noexcept -> void
{
  m_filterViewport = val;
}

inline auto ZoomFilterBufferStriper::UpdateAllStripes() noexcept -> void
{
  DoNextStripe(m_dimensions.GetHeight());
}

inline auto ZoomFilterBufferStriper::UpdateNextStripe() noexcept -> void
{
  DoNextStripe(m_tranBuffStripeHeight);
}

} // namespace GOOM::FILTER_FX
