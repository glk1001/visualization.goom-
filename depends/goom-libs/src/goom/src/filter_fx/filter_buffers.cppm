module;

#include <algorithm>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <span>
#include <vector>

export module Goom.FilterFx.FilterBuffers;

import Goom.FilterFx.NormalizedCoords;
import Goom.Utils.Parallel;
import Goom.Lib.AssertUtils;
import Goom.Lib.GoomTypes;
import Goom.Lib.Point2d;
import Goom.PluginInfo;

export namespace GOOM::FILTER_FX
{

class ZoomFilterBuffers
{
public:
  enum class UpdateStatus : UnderlyingEnumType
  {
    AT_START,
    IN_PROGRESS,
    AT_END,
    HAS_BEEN_COPIED,
  };

  using ZoomPointFunc = std::function<NormalizedCoords(const NormalizedCoords& normalizedCoords)>;

  ZoomFilterBuffers(const PluginInfo& goomInfo,
                    const NormalizedCoordsConverter& normalizedCoordsConverter,
                    const ZoomPointFunc& getZoomPointFunc) noexcept;

  auto SetTransformBufferMidpoint(const Point2dInt& midpoint) noexcept -> void;

  auto Start() noexcept -> void;
  auto Finish() noexcept -> void;

  auto TransformBufferThread() noexcept -> void;

  [[nodiscard]] auto GetUpdateStatus() const noexcept -> UpdateStatus;
  auto ResetTransformBufferToStart() noexcept -> void;
  auto StartTransformBufferUpdates() noexcept -> void;

  auto CopyTransformBuffer(std::span<Point2dFlt> destBuff) noexcept -> void;

protected:
  // For testing only.
  [[nodiscard]] auto GetTransformBufferMidpoint() const noexcept -> Point2dInt;
  auto UpdateTransformBuffer() noexcept -> void;

private:
  Dimensions m_dimensions;
  const NormalizedCoordsConverter* m_normalizedCoordsConverter;
  UpdateStatus m_updateStatus = UpdateStatus::AT_START;

  bool m_shutdown = false;
  std::mutex m_mutex;
  std::condition_variable m_bufferProducer_cv;

  UTILS::Parallel m_parallel{UTILS::GetNumAvailablePoolThreads()};
  ZoomPointFunc m_getZoomPoint;
  Point2dInt m_midpoint                 = {.x = 0, .y = 0};
  NormalizedCoords m_normalizedMidpoint = {0.0F, 0.0F};

  std::vector<Point2dFlt> m_transformBuffer;

  auto DoNextTransformBuffer() noexcept -> void;
};

} // namespace GOOM::FILTER_FX

namespace GOOM::FILTER_FX
{

inline auto ZoomFilterBuffers::GetUpdateStatus() const noexcept -> UpdateStatus
{
  return m_updateStatus;
}

inline auto ZoomFilterBuffers::GetTransformBufferMidpoint() const noexcept -> Point2dInt
{
  return m_midpoint;
}

inline auto ZoomFilterBuffers::SetTransformBufferMidpoint(const Point2dInt& midpoint) noexcept
    -> void
{
  Expects(UpdateStatus::AT_START == m_updateStatus);

  m_midpoint           = midpoint;
  m_normalizedMidpoint = m_normalizedCoordsConverter->OtherToNormalizedCoords(m_midpoint);
}

inline auto ZoomFilterBuffers::CopyTransformBuffer(std::span<Point2dFlt> destBuff) noexcept -> void
{
  Expects(UpdateStatus::AT_END == m_updateStatus);

  std::ranges::copy(m_transformBuffer, destBuff.begin());
  m_updateStatus = UpdateStatus::HAS_BEEN_COPIED;
}

} // namespace GOOM::FILTER_FX
