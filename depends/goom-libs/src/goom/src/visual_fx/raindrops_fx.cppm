module;

#include "goom/point2d.h"
#include "goom/spimpl.h"

#include <string>
#include <vector>

export module Goom.VisualFx.RaindropsFx;

import Goom.VisualFx.VisualFxBase;

export namespace GOOM::VISUAL_FX
{
class FxHelper;

class RaindropsFx : public IVisualFx
{
public:
  explicit RaindropsFx(FxHelper& fxHelper) noexcept;

  [[nodiscard]] auto GetFxName() const noexcept -> std::string override;

  auto Start() noexcept -> void override;
  auto Finish() noexcept -> void override;

  auto ChangePixelBlender(const PixelBlenderParams& pixelBlenderParams) noexcept -> void override;
  auto SetZoomMidpoint(const Point2dInt& zoomMidpoint) noexcept -> void override;

  auto SetWeightedColorMaps(const WeightedColorMaps& weightedColorMaps) noexcept -> void override;
  [[nodiscard]] auto GetCurrentColorMapsNames() const noexcept -> std::vector<std::string> override;

  auto ApplyToImageBuffers() noexcept -> void override;

private:
  class RaindropsFxImpl;
  spimpl::unique_impl_ptr<RaindropsFxImpl> m_pimpl;
};

} // namespace GOOM::VISUAL_FX
