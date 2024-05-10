module;

#include "goom/goom_config.h"

#include <string>
#include <vector>

export module Goom.VisualFx.ParticlesFx;

import Goom.Color.RandomColorMaps;
import Goom.Utils.Graphics.SmallImageBitmaps;
import Goom.VisualFx.FxHelper;
import Goom.VisualFx.VisualFxBase;
import Goom.Lib.Point2d;
import Goom.Lib.SPimpl;

export namespace GOOM::VISUAL_FX
{

class ParticlesFx : public IVisualFx
{
public:
  ParticlesFx(FxHelper& fxHelper, const UTILS::GRAPHICS::SmallImageBitmaps& smallBitmaps) noexcept;

  [[nodiscard]] auto GetFxName() const noexcept -> std::string override;

  auto Start() noexcept -> void override;
  auto Finish() noexcept -> void override;

  auto ChangePixelBlender(const PixelBlenderParams& pixelBlenderParams) noexcept -> void override;
  auto SetZoomMidpoint(const Point2dInt& zoomMidpoint) noexcept -> void override;

  [[nodiscard]] auto GetCurrentColorMapsNames() const noexcept -> std::vector<std::string> override;
  auto SetWeightedColorMaps(const WeightedColorMaps& weightedColorMaps) noexcept -> void override;

  auto ApplyToImageBuffers() noexcept -> void override;

private:
  class ParticlesFxImpl;
  spimpl::unique_impl_ptr<ParticlesFxImpl> m_pimpl;
};

} // namespace GOOM::VISUAL_FX
