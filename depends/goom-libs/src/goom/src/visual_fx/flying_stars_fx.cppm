module;

#include "goom/goom_config.h"

#include <string>
#include <vector>

export module Goom.VisualFx.FlyingStarsFx;

export import :StarTypesContainer;

import Goom.Utils.Graphics.SmallImageBitmaps;
import Goom.VisualFx.FxHelper;
import Goom.VisualFx.VisualFxBase;
import Goom.Lib.Point2d;
import Goom.Lib.SPimpl;

export namespace GOOM::VISUAL_FX
{

class FlyingStarsFx : public IVisualFx
{
public:
  FlyingStarsFx() noexcept = delete;
  FlyingStarsFx(FxHelper& fxHelper,
                const UTILS::GRAPHICS::SmallImageBitmaps& smallBitmaps) noexcept;

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
  class FlyingStarsImpl;
  spimpl::unique_impl_ptr<FlyingStarsImpl> m_pimpl;
};

} // namespace GOOM::VISUAL_FX
