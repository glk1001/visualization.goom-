#pragma once

#include "goom_types.h"
#include "normalized_coords.h"
#include "point2d.h"
#include "utils/math/misc.h"

#include <cstdint>
#include <memory>
#include <span>

namespace GOOM::FILTER_FX
{

namespace FILTER_BUFFERS
{
// TODO - Is this necessary?
inline constexpr float MIN_SCREEN_COORD_ABS_VAL = 1.0F / 16.0F;
} // namespace FILTER_BUFFERS

template<class FilterStriper>
class ZoomFilterBuffers
{
public:
  enum class TransformBufferState
  {
    START_FRESH_TRANSFORM_BUFFER,
    RESET_TRANSFORM_BUFFER,
    TRANSFORM_BUFFER_READY,
  };

  explicit ZoomFilterBuffers(std::unique_ptr<FilterStriper> filterStriper) noexcept;

  auto Start() noexcept -> void;

  [[nodiscard]] auto GetTransformBufferBuffMidpoint() const noexcept -> Point2dInt;
  auto SetTransformBufferMidpoint(const Point2dInt& val) noexcept -> void;

  [[nodiscard]] auto GetFilterViewport() const noexcept -> Viewport;
  auto SetFilterViewport(const Viewport& val) noexcept -> void;

  [[nodiscard]] auto GetTransformBufferYLineStart() const noexcept -> uint32_t;

  auto NotifyFilterSettingsHaveChanged() noexcept -> void;
  [[nodiscard]] auto HaveFilterSettingsChanged() const noexcept -> bool;

  [[nodiscard]] auto IsTransformBufferReady() const noexcept -> bool;
  auto CopyTransformBuffer(std_spn::span<Point2dFlt>& destBuff) noexcept -> void;

  auto UpdateTransformBuffer() noexcept -> void;
  [[nodiscard]] auto GetTransformBufferState() const noexcept -> TransformBufferState;

  static auto UpdateSrcePosFilterBuffer(const float transformBufferLerpFactor,
                                        const std_spn::span<Point2dFlt>& destFilterPosBuffer,
                                        std_spn::span<Point2dFlt> srceFilterPosBuffer) noexcept
      -> void;

private:
  std::unique_ptr<FilterStriper> m_filterStriper;

  bool m_filterSettingsHaveChanged            = false;
  TransformBufferState m_transformBufferState = TransformBufferState::TRANSFORM_BUFFER_READY;

  auto InitTransformBuffer() noexcept -> void;
  auto StartFreshTranBuffer() noexcept -> void;
  auto ResetTransformBuffer() noexcept -> void;
  auto UpdateNextTransformBufferStripe() noexcept -> void;
  auto FillTransformBuffer() noexcept -> void;
};

template<class FilterStriper>
ZoomFilterBuffers<FilterStriper>::ZoomFilterBuffers(
    std::unique_ptr<FilterStriper> filterStriper) noexcept
  : m_filterStriper{std::move(filterStriper)}
{
}

template<class FilterStriper>
inline auto ZoomFilterBuffers<FilterStriper>::IsTransformBufferReady() const noexcept -> bool
{
  return m_filterStriper->IsTransformBufferReady();
}

template<class FilterStriper>
inline auto ZoomFilterBuffers<FilterStriper>::CopyTransformBuffer(
    std_spn::span<Point2dFlt>& destBuff) noexcept -> void
{
  m_filterStriper->CopyTransformBuffer(destBuff);
}

template<class FilterStriper>
inline auto ZoomFilterBuffers<FilterStriper>::GetTransformBufferBuffMidpoint() const noexcept
    -> Point2dInt
{
  return m_filterStriper->GetTransformBufferMidpoint();
}

template<class FilterStriper>
inline auto ZoomFilterBuffers<FilterStriper>::SetTransformBufferMidpoint(
    const Point2dInt& val) noexcept -> void
{
  m_filterStriper->SetTransformBufferMidpoint(val);
}

template<class FilterStriper>
inline auto ZoomFilterBuffers<FilterStriper>::GetFilterViewport() const noexcept -> Viewport
{
  return m_filterStriper->GetFilterViewport();
}

template<class FilterStriper>
inline auto ZoomFilterBuffers<FilterStriper>::SetFilterViewport(const Viewport& val) noexcept
    -> void
{
  m_filterStriper->SetFilterViewport(val);
}

template<class FilterStriper>
inline auto ZoomFilterBuffers<FilterStriper>::GetTransformBufferState() const noexcept
    -> TransformBufferState
{
  return m_transformBufferState;
}

template<class FilterStriper>
inline auto ZoomFilterBuffers<FilterStriper>::HaveFilterSettingsChanged() const noexcept -> bool
{
  return m_filterSettingsHaveChanged;
}

template<class FilterStriper>
inline auto ZoomFilterBuffers<FilterStriper>::NotifyFilterSettingsHaveChanged() noexcept -> void
{
  m_filterSettingsHaveChanged = true;
}

template<class FilterStriper>
inline auto ZoomFilterBuffers<FilterStriper>::GetTransformBufferYLineStart() const noexcept
    -> uint32_t
{
  return m_filterStriper->GetTransformBufferYLineStart();
}

template<class FilterStriper>
inline auto ZoomFilterBuffers<FilterStriper>::Start() noexcept -> void
{
  InitTransformBuffer();
}

template<class FilterStriper>
inline auto ZoomFilterBuffers<FilterStriper>::InitTransformBuffer() noexcept -> void
{
  m_transformBufferState = TransformBufferState::TRANSFORM_BUFFER_READY;
  m_filterStriper->ResetStripes();

  FillTransformBuffer();

  m_filterStriper->ResetStripes();
  m_transformBufferState = TransformBufferState::START_FRESH_TRANSFORM_BUFFER;
}

template<class FilterStriper>
inline auto ZoomFilterBuffers<FilterStriper>::UpdateTransformBuffer() noexcept -> void
{
  if (m_transformBufferState == TransformBufferState::RESET_TRANSFORM_BUFFER)
  {
    ResetTransformBuffer();
  }
  else if (m_transformBufferState == TransformBufferState::START_FRESH_TRANSFORM_BUFFER)
  {
    StartFreshTranBuffer();
  }
  else
  {
    UpdateNextTransformBufferStripe();
  }
}

template<class FilterStriper>
inline auto ZoomFilterBuffers<FilterStriper>::StartFreshTranBuffer() noexcept -> void
{
  if (not m_filterSettingsHaveChanged)
  {
    return;
  }

  m_filterSettingsHaveChanged = false;
  m_filterStriper->ResetStripes();
  m_filterStriper->ResetTransformBufferIsReadyFlag();
  m_transformBufferState = TransformBufferState::TRANSFORM_BUFFER_READY;
}

template<class FilterStriper>
inline auto ZoomFilterBuffers<FilterStriper>::ResetTransformBuffer() noexcept -> void
{
  m_filterStriper->ResetStripes();
  m_transformBufferState = TransformBufferState::START_FRESH_TRANSFORM_BUFFER;
}

template<class FilterStriper>
inline auto ZoomFilterBuffers<FilterStriper>::UpdateNextTransformBufferStripe() noexcept -> void
{
  m_filterStriper->UpdateNextStripe();
  if (0 == m_filterStriper->GetTransformBufferYLineStart())
  {
    m_transformBufferState = TransformBufferState::RESET_TRANSFORM_BUFFER;
  }
}

template<class FilterStriper>
inline auto ZoomFilterBuffers<FilterStriper>::FillTransformBuffer() noexcept -> void
{
  m_filterStriper->UpdateAllStripes();
}

template<class FilterStriper>
auto ZoomFilterBuffers<FilterStriper>::UpdateSrcePosFilterBuffer(
    const float transformBufferLerpFactor,
    const std_spn::span<Point2dFlt>& destFilterPosBuffer,
    std_spn::span<Point2dFlt> srceFilterPosBuffer) noexcept -> void
{
  std::transform(destFilterPosBuffer.begin(),
                 destFilterPosBuffer.end(),
                 srceFilterPosBuffer.begin(),
                 srceFilterPosBuffer.begin(),
                 [&transformBufferLerpFactor](const Point2dFlt& destPos, const Point2dFlt& srcePos)
                 { return lerp(srcePos, destPos, transformBufferLerpFactor); });
}

} // namespace GOOM::FILTER_FX
