module;

#include "goom/goom_config.h"

#include <cstdint>
#include <string>
#include <vector>

namespace GOOM::COLOR
{
class RandomColorMaps;
}

export module Goom.VisualFx.ShapesFx;

import Goom.VisualFx.VisualFxBase;
import Goom.Lib.Point2d;
import Goom.Lib.SPimpl;

export namespace GOOM::VISUAL_FX
{
class FxHelper;

class ShapesFx : public IVisualFx
{
public:
  static constexpr uint32_t NUM_SHAPES = 1;

  explicit ShapesFx(FxHelper& fxHelper) noexcept;

  [[nodiscard]] auto GetFxName() const noexcept -> std::string override;

  auto Start() noexcept -> void override;
  auto Finish() noexcept -> void override;

  auto ChangePixelBlender(const PixelBlenderParams& pixelBlenderParams) noexcept -> void override;
  auto SetZoomMidpoint(const Point2dInt& zoomMidpoint) noexcept -> void override;

  auto SetWeightedColorMaps(const WeightedColorMaps& weightedColorMaps) noexcept -> void override;
  [[nodiscard]] auto GetCurrentColorMapsNames() const noexcept -> std::vector<std::string> override;

  auto ApplyToImageBuffers() noexcept -> void override;

private:
  class ShapesFxImpl;
  spimpl::unique_impl_ptr<ShapesFxImpl> m_pimpl;
};

} // namespace GOOM::VISUAL_FX
