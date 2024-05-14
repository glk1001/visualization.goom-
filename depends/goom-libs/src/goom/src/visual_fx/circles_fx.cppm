module;

#include "goom/point2d.h"
#include "goom/spimpl.h"

#include <string>
#include <vector>

export module Goom.VisualFx.CirclesFx;

import Goom.Utils.Graphics.SmallImageBitmaps;
import Goom.VisualFx.VisualFxBase;

export namespace GOOM::VISUAL_FX
{
class FxHelper;

class CirclesFx : public IVisualFx
{
public:
  CirclesFx(FxHelper& fxHelper, const UTILS::GRAPHICS::SmallImageBitmaps& smallBitmaps) noexcept;

  [[nodiscard]] auto GetFxName() const noexcept -> std::string override;

  auto Start() noexcept -> void override;
  auto Finish() noexcept -> void override;

  auto ChangePixelBlender(const PixelBlenderParams& pixelBlenderParams) noexcept -> void override;
  auto SetZoomMidpoint(const Point2dInt& zoomMidpoint) noexcept -> void override;

  [[nodiscard]] auto GetCurrentColorMapsNames() const noexcept -> std::vector<std::string> override;
  auto SetWeightedColorMaps(const WeightedColorMaps& weightedColorMaps) noexcept -> void override;

  auto ApplyToImageBuffers() noexcept -> void override;

private:
  class CirclesFxImpl;
  spimpl::unique_impl_ptr<CirclesFxImpl> m_pimpl;
};

} // namespace GOOM::VISUAL_FX
