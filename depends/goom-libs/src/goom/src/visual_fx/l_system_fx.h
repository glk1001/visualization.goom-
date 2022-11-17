#pragma once

#include "goom_visual_fx.h"
#include "spimpl.h"

#include <string>
#include <vector>

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

class LSystemFx : public IVisualFx
{
public:
  LSystemFx() noexcept = delete;
  LSystemFx(const FxHelper& fxHelper,
            const UTILS::GRAPHICS::SmallImageBitmaps& smallBitmaps,
            const std::string& resourcesDirectory) noexcept;

  [[nodiscard]] auto GetFxName() const noexcept -> std::string override;

  auto Start() noexcept -> void override;
  auto Finish() noexcept -> void override;

  auto Resume() noexcept -> void override;
  auto Suspend() noexcept -> void override;

  auto SetWeightedColorMaps(const WeightedColorMaps& weightedColorMaps) noexcept -> void override;
  auto SetZoomMidpoint(const Point2dInt& zoomMidpoint) noexcept -> void override;

  [[nodiscard]] auto GetCurrentColorMapsNames() const noexcept -> std::vector<std::string> override;

  auto ApplyMultiple() noexcept -> void override;

private:
  class LSystemFxImpl;
  spimpl::unique_impl_ptr<LSystemFxImpl> m_pimpl;
};

} // namespace VISUAL_FX
} // namespace GOOM
