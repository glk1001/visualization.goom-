#pragma once

#include "goom/spimpl.h"
#include "goom_visual_fx.h"

#include <memory>

namespace GOOM
{

namespace COLOR
{
class RandomColorMaps;
}

namespace UTILS::GRAPHICS
{
class SmallImageBitmaps;
}

namespace VISUAL_FX
{
class FxHelper;

class TubesFx : public IVisualFx
{
public:
  TubesFx() noexcept = delete;
  TubesFx(const FxHelper& fxHelper,
          const UTILS::GRAPHICS::SmallImageBitmaps& smallBitmaps) noexcept;

  [[nodiscard]] auto GetFxName() const noexcept -> std::string override;

  auto Start() noexcept -> void override;
  auto Finish() noexcept -> void override;

  auto Resume() noexcept -> void override;
  auto Suspend() noexcept -> void override;

  auto SetWeightedColorMaps(const WeightedColorMaps& weightedColorMaps) noexcept -> void override;
  auto SetZoomMidpoint(const Point2dInt& zoomMidpoint) noexcept -> void override;

  auto ApplyMultiple() noexcept -> void override;

private:
  class TubeFxImpl;
  spimpl::unique_impl_ptr<TubeFxImpl> m_fxImpl;
};

} // namespace VISUAL_FX
} // namespace GOOM
