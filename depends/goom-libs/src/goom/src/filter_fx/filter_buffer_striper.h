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

  auto SetFilterViewport(const Viewport& viewport) noexcept -> void;

  auto ResetStripes() noexcept -> void;
  auto ResetTransformBufferIsReadyFlag() noexcept -> void;

  auto FillTransformBuffer() noexcept -> void;
  auto UpdateNextStripe() noexcept -> void;

  [[nodiscard]] auto IsTransformBufferReady() const noexcept -> bool;
  [[nodiscard]] auto GetPreviousTransformBuffer() const noexcept -> const std::vector<Point2dFlt>&;
  auto CopyTransformBuffer(std_spn::span<Point2dFlt>& destBuff) noexcept -> void;
  auto RestartTransformBuffer() noexcept -> void;

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
  std::vector<Point2dFlt> m_transformBuffer;
  std::vector<Point2dFlt> m_previousTransformBuffer;
  bool m_transformBufferIsReady = false;

  auto UpdateAllStripes() noexcept -> void;
  auto DoNextStripe(uint32_t transformBufferStripeHeight) noexcept -> void;
  auto SwapToPreviousTransformBuffer() noexcept -> void;
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
  Expects(m_transformBufferIsReady);
  std::copy(m_transformBuffer.cbegin(), m_transformBuffer.cend(), destBuff.begin());
}

inline auto ZoomFilterBufferStriper::RestartTransformBuffer() noexcept -> void
{
  Expects(m_transformBufferIsReady);
  SwapToPreviousTransformBuffer();
  m_transformBufferIsReady = false;
}

inline auto ZoomFilterBufferStriper::GetTransformBufferMidpoint() const noexcept -> Point2dInt
{
  return m_midpoint;
}

inline auto ZoomFilterBufferStriper::SetTransformBufferMidpoint(const Point2dInt& midpoint) noexcept
    -> void
{
  Expects(m_transformBufferYLineStart == 0U);
  m_midpoint           = midpoint;
  m_normalizedMidpoint = m_normalizedCoordsConverter->OtherToNormalizedCoords(m_midpoint);
}

inline auto ZoomFilterBufferStriper::SetFilterViewport(const Viewport& viewport) noexcept -> void
{
  Expects(m_transformBufferYLineStart == 0U);
  m_filterViewport = viewport;
}

inline auto ZoomFilterBufferStriper::FillTransformBuffer() noexcept -> void
{
  ResetStripes();
  UpdateAllStripes();
  SwapToPreviousTransformBuffer();
  ResetTransformBufferIsReadyFlag();
  UpdateAllStripes();
}

inline auto ZoomFilterBufferStriper::UpdateAllStripes() noexcept -> void
{
  DoNextStripe(m_dimensions.GetHeight());
}

inline auto ZoomFilterBufferStriper::UpdateNextStripe() noexcept -> void
{
  DoNextStripe(m_transformBufferStripeHeight);
}

inline auto ZoomFilterBufferStriper::GetPreviousTransformBuffer() const noexcept
    -> const std::vector<Point2dFlt>&
{
  return m_previousTransformBuffer;
}

inline auto ZoomFilterBufferStriper::SwapToPreviousTransformBuffer() noexcept -> void
{
  std::swap(m_previousTransformBuffer, m_transformBuffer);
}

} // namespace GOOM::FILTER_FX
