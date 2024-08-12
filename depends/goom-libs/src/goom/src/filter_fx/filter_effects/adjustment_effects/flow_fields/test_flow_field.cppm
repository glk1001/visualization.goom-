module;

#include <array>
//#include <mdspan.hpp>

export module Goom.FilterFx.FilterEffects.AdjustmentEffects.TestFlowField;

import Goom.FilterFx.FilterUtils.Utils;
import Goom.FilterFx.CommonTypes;
import Goom.FilterFx.NormalizedCoords;
import Goom.FilterFx.ZoomAdjustmentEffect;
import Goom.Utils.NameValuePairs;
import Goom.Utils.Math.GoomRand;
import Goom.Lib.Point2d;

using GOOM::FILTER_FX::FILTER_UTILS::LerpToOneTs;
using GOOM::UTILS::MATH::GoomRand;

export namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

class TestFlowField
{
public:
  explicit TestFlowField(const GoomRand& goomRand) noexcept;

  auto SetRandomParams() noexcept -> void;

  [[nodiscard]] auto GetZoomAdjustment(const Vec2dFlt& baseZoomAdjustment,
                                       const NormalizedCoords& coords) const noexcept -> Vec2dFlt;

  [[nodiscard]] auto GetZoomAdjustmentEffectNameValueParams() const noexcept
      -> UTILS::NameValuePairs;

  struct Params
  {
    Amplitude amplitude{};
    LerpToOneTs lerpToOneTs{};
    FrequencyFactor angleFrequencyFactor{};
  };
  [[nodiscard]] auto GetParams() const noexcept -> const Params&;

protected:
  auto SetParams(const Params& params) noexcept -> void;

private:
  const GoomRand* m_goomRand;
  static constexpr auto GRID_HEIGHT = 500UL;
  static constexpr auto GRID_WIDTH  = 500UL;
  std::array<float, GRID_WIDTH * GRID_HEIGHT> m_gridArray{};
  // std::mdspan<float, std::extents<unsigned long, GRID_HEIGHT, GRID_WIDTH>>
  //    gridAngles{m_gridArray.data()};
  NormalizedCoordsConverter m_normalizedCoordsToGridConverter{
      {GRID_WIDTH, GRID_HEIGHT}
  };
  auto SetupAngles() noexcept -> void;
  Params m_params;
  [[nodiscard]] auto GetVelocity(const Vec2dFlt& baseZoomAdjustment,
                                 const NormalizedCoords& coords) const noexcept -> Vec2dFlt;
};

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

inline auto TestFlowField::GetZoomAdjustment(
    const Vec2dFlt& baseZoomAdjustment, const NormalizedCoords& coords) const noexcept -> Vec2dFlt
{
  const auto velocity = GetVelocity(baseZoomAdjustment, coords);

  return GetVelocityByZoomLerpedToOne(coords, m_params.lerpToOneTs, velocity);
}

inline auto TestFlowField::GetParams() const noexcept -> const Params&
{
  return m_params;
}

inline void TestFlowField::SetParams(const Params& params) noexcept
{
  m_params = params;
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
