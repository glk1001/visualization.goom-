#pragma once

#include "filter_fx/normalized_coords.h"
#include "goom/point2d.h"
#include "utils/name_value_pairs.h"

namespace GOOM::FILTER_FX
{

class IZoomAdjustmentEffect
{
public:
  IZoomAdjustmentEffect() noexcept                                       = default;
  IZoomAdjustmentEffect(const IZoomAdjustmentEffect&) noexcept           = default;
  IZoomAdjustmentEffect(IZoomAdjustmentEffect&&) noexcept                = default;
  virtual ~IZoomAdjustmentEffect() noexcept                              = default;
  auto operator=(const IZoomAdjustmentEffect&) -> IZoomAdjustmentEffect& = default;
  auto operator=(IZoomAdjustmentEffect&&) -> IZoomAdjustmentEffect&      = default;

  virtual auto SetRandomParams() noexcept -> void = 0;
  [[nodiscard]] virtual auto GetZoomAdjustmentViewport() const noexcept -> Viewport;

  auto SetBaseZoomAdjustment(const Point2dFlt& baseZoomAdjustment) noexcept -> void;

  [[nodiscard]] virtual auto GetZoomAdjustment(const NormalizedCoords& coords,
                                               float sqDistFromZero) const noexcept
      -> Point2dFlt = 0;

  [[nodiscard]] virtual auto GetZoomAdjustmentEffectNameValueParams() const noexcept
      -> GOOM::UTILS::NameValuePairs = 0;

protected:
  static constexpr auto* PARAM_GROUP = "Zoom Adj";
  [[nodiscard]] auto GetBaseZoomAdjustment() const noexcept -> const Point2dFlt&;

private:
  Point2dFlt m_baseZoomAdjustment{};
};

inline auto IZoomAdjustmentEffect::GetZoomAdjustmentViewport() const noexcept -> Viewport
{
  return Viewport{};
}

inline auto IZoomAdjustmentEffect::GetBaseZoomAdjustment() const noexcept -> const Point2dFlt&
{
  return m_baseZoomAdjustment;
}

inline auto IZoomAdjustmentEffect::SetBaseZoomAdjustment(
    const Point2dFlt& baseZoomAdjustment) noexcept -> void
{
  m_baseZoomAdjustment = baseZoomAdjustment;
}

} // namespace GOOM::FILTER_FX
