#pragma once

#include "goom/spimpl.h"
#include "goom_visual_fx.h"

#include <string>
#include <vector>

namespace GOOM::COLOR
{
class RandomColorMaps;
}
namespace GOOM::UTILS::GRAPHICS
{
class SmallImageBitmaps;
}

namespace GOOM::VISUAL_FX
{
class FxHelper;

class LSystemFx : public IVisualFx
{
public:
  LSystemFx() noexcept = delete;
  LSystemFx(FxHelper& fxHelper, const std::string& resourcesDirectory) noexcept;

  [[nodiscard]] auto GetFxName() const noexcept -> std::string override;

  auto Start() noexcept -> void override;
  auto Finish() noexcept -> void override;

  auto Resume() noexcept -> void override;
  auto Suspend() noexcept -> void override;

  auto ChangePixelBlender(const PixelBlenderParams& pixelBlenderParams) noexcept -> void override;
  auto SetZoomMidpoint(const Point2dInt& zoomMidpoint) noexcept -> void override;

  auto SetWeightedColorMaps(const WeightedColorMaps& weightedColorMaps) noexcept -> void override;
  [[nodiscard]] auto GetCurrentColorMapsNames() const noexcept -> std::vector<std::string> override;

  auto ApplyToImageBuffers() noexcept -> void override;

private:
  class LSystemFxImpl;
  spimpl::unique_impl_ptr<LSystemFxImpl> m_pimpl;
};

} // namespace GOOM::VISUAL_FX
