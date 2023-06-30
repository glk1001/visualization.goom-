#pragma once

#include "goom_types.h"
#include "zoom_coord_transforms.h"

#include <cstdint>

namespace GOOM::FILTER_FX::FILTER_UTILS
{

class ZoomTransformBuffers
{
  static constexpr auto MAX_TRAN_LERP_EXP = 16U;

public:
  static constexpr auto MAX_TRAN_LERP_VALUE = UTILS::MATH::PowerOf2(MAX_TRAN_LERP_EXP);

  ZoomTransformBuffers(const Dimensions& dimensions) noexcept;

  [[nodiscard]] auto GetTranLerpFactor() const noexcept -> uint32_t;
  auto SetTranLerpFactor(uint32_t val) noexcept -> void;

private:
  Dimensions m_dimensions;
  uint32_t m_tranLerpFactor = 0U;
};

inline ZoomTransformBuffers::ZoomTransformBuffers(const Dimensions& dimensions) noexcept
  : m_dimensions{dimensions}
{
}

inline auto ZoomTransformBuffers::GetTranLerpFactor() const noexcept -> uint32_t
{
  return m_tranLerpFactor;
}

inline auto ZoomTransformBuffers::SetTranLerpFactor(const uint32_t val) noexcept -> void
{
  m_tranLerpFactor = val;
}

} // namespace GOOM::FILTER_FX::FILTER_UTILS
