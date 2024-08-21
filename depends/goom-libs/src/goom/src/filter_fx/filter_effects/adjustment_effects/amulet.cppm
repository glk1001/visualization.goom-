export module Goom.FilterFx.FilterEffects.AdjustmentEffects.Amulet;

import Goom.FilterFx.FilterUtils.Utils;
import Goom.FilterFx.CommonTypes;
import Goom.FilterFx.NormalizedCoords;
import Goom.FilterFx.ZoomAdjustmentEffect;
import Goom.Utils.NameValuePairs;
import Goom.Utils.Math.GoomRand;
import Goom.Lib.Point2d;

using GOOM::UTILS::NameValuePairs;
using GOOM::UTILS::MATH::GoomRand;

export namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

class Amulet : public IZoomAdjustmentEffect
{
public:
  explicit Amulet(const GoomRand& goomRand) noexcept;

  auto SetRandomParams() noexcept -> void override;

  [[nodiscard]] auto GetZoomAdjustment(const NormalizedCoords& coords) const noexcept
      -> Vec2dFlt override;

  [[nodiscard]] auto GetZoomAdjustmentEffectNameValueParams() const noexcept
      -> NameValuePairs override;

  struct Params
  {
    Viewport viewport;
    Amplitude amplitude{};
  };
  [[nodiscard]] auto GetParams() const noexcept -> const Params&;

protected:
  auto SetParams(const Params& params) noexcept -> void;

private:
  const GoomRand* m_goomRand;
  FILTER_UTILS::RandomViewport m_randomViewport;
  Params m_params;
  [[nodiscard]] auto GetRandomParams() const noexcept -> Params;
  [[nodiscard]] auto GetVelocity(const NormalizedCoords& coords) const noexcept -> Vec2dFlt;
};

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

inline auto Amulet::GetZoomAdjustment(const NormalizedCoords& coords) const noexcept -> Vec2dFlt
{
  const auto velocity = GetVelocity(coords);

  return {.x = coords.GetX() * velocity.x, .y = coords.GetY() * velocity.y};
}

inline auto Amulet::GetParams() const noexcept -> const Params&
{
  return m_params;
}

inline void Amulet::SetParams(const Params& params) noexcept
{
  m_params = params;
}

inline auto Amulet::SetRandomParams() noexcept -> void
{
  m_params = GetRandomParams();
}

inline auto Amulet::GetVelocity(const NormalizedCoords& coords) const noexcept -> Vec2dFlt
{
  const auto viewportCoords = m_params.viewport.GetViewportCoords(coords);

  const auto sqDistFromZero = SqDistanceFromZero(viewportCoords);

  return {.x = GetBaseZoomAdjustment().x + (m_params.amplitude.x * sqDistFromZero),
          .y = GetBaseZoomAdjustment().y + (m_params.amplitude.y * sqDistFromZero)};
  //?      speedCoeffs.y = 5.0F * std::cos(5.0F * speedCoeffs.x) * std::sin(5.0F * speedCoeffs.y);
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
