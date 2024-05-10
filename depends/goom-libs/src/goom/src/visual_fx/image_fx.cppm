module;

#include "goom/goom_config.h"

#include <string>
#include <vector>

export module Goom.VisualFx.ImageFx;

import Goom.Color.RandomColorMaps;
import Goom.Utils.Parallel;
import Goom.VisualFx.FxHelper;
import Goom.VisualFx.VisualFxBase;
import Goom.Lib.SPimpl;

export namespace GOOM::VISUAL_FX
{

class ImageFx : public IVisualFx
{
public:
  ImageFx(UTILS::Parallel& parallel,
          FxHelper& fxHelper,
          const std::string& resourcesDirectory) noexcept;

  [[nodiscard]] auto GetFxName() const noexcept -> std::string override;

  auto Start() noexcept -> void override;
  auto Finish() noexcept -> void override;

  auto Resume() noexcept -> void override;

  auto ChangePixelBlender(const PixelBlenderParams& pixelBlenderParams) noexcept -> void override;

  auto SetWeightedColorMaps(const WeightedColorMaps& weightedColorMaps) noexcept -> void override;
  [[nodiscard]] auto GetCurrentColorMapsNames() const noexcept -> std::vector<std::string> override;

  auto ApplyToImageBuffers() noexcept -> void override;

private:
  class ImageFxImpl;
  spimpl::unique_impl_ptr<ImageFxImpl> m_pimpl;
};

} // namespace GOOM::VISUAL_FX
