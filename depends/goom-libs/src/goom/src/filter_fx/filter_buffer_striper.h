#pragma once

#include "goom_graphic.h"
#include "goom_types.h"
#include "normalized_coords.h"
#include "point2d.h"

#include <algorithm>
#include <cstdint>
#include <functional>
#include <span>
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

  [[nodiscard]] auto GetTransformBufferMidpoint() const noexcept -> Point2dInt;
  auto SetTransformBufferMidpoint(const Point2dInt& midpoint) noexcept -> void;

  [[nodiscard]] auto GetFilterViewport() const noexcept -> Viewport;
  auto SetFilterViewport(const Viewport& val) noexcept -> void;

  auto ResetStripes() noexcept -> void;
  auto ResetTransformBufferIsReadyFlag() noexcept -> void;

  auto UpdateAllStripes() noexcept -> void;
  auto UpdateNextStripe() noexcept -> void;

  [[nodiscard]] auto IsTransformBufferReady() const noexcept -> bool;
  auto CopyTransformBuffer(std_spn::span<Point2dFlt>& destBuff) noexcept -> void;

  [[nodiscard]] auto GetTransformBufferYLineStart() const noexcept -> uint32_t;

private:
  Dimensions m_dimensions;
  const NormalizedCoordsConverter* m_normalizedCoordsConverter;

  UTILS::Parallel* m_parallel;
  ZoomPointFunc m_getZoomPoint;
  Point2dInt m_midpoint                 = {0, 0};
  NormalizedCoords m_normalizedMidpoint = {0.0F, 0.0F};
  Viewport m_filterViewport             = Viewport{};

  // 'NUM_STRIPE_GROUPS' controls how many updates before all stripes, and therefore,
  // all of the tran buffer, is filled. We use stripes to spread the buffer update load
  // over a number of updates. Too few and performance suffers periodically for a
  // number of updates; too many, and performance suffers overall.
  static constexpr auto NUM_STRIPE_GROUPS = 20U;
  uint32_t m_transformBufferYLineStart    = 0;
  uint32_t m_transformBufferStripeHeight  = m_dimensions.GetHeight() / NUM_STRIPE_GROUPS;
  std::vector<Point2dFlt> m_transformBuffer{};
  bool m_transformBufferIsReady = false;

  auto DoNextStripe(uint32_t transformBufferStripeHeight) noexcept -> void;
};

inline auto ZoomFilterBufferStriper::GetTransformBufferYLineStart() const noexcept -> uint32_t
{
  return m_transformBufferYLineStart;
}

inline auto ZoomFilterBufferStriper::IsTransformBufferReady() const noexcept -> bool
{
  return m_transformBufferIsReady;
}

inline auto ZoomFilterBufferStriper::CopyTransformBuffer(
    std_spn::span<Point2dFlt>& destBuff) noexcept -> void
{
  std::copy(m_transformBuffer.cbegin(), m_transformBuffer.cend(), destBuff.begin());
  m_transformBufferIsReady = false;
}

inline auto ZoomFilterBufferStriper::GetTransformBufferMidpoint() const noexcept -> Point2dInt
{
  return m_midpoint;
}

inline auto ZoomFilterBufferStriper::SetTransformBufferMidpoint(const Point2dInt& midpoint) noexcept
    -> void
{
  m_midpoint           = midpoint;
  m_normalizedMidpoint = m_normalizedCoordsConverter->OtherToNormalizedCoords(m_midpoint);
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
  DoNextStripe(m_transformBufferStripeHeight);
}

} // namespace GOOM::FILTER_FX
