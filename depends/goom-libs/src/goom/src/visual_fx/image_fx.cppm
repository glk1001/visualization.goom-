module;

#include "color/random_color_maps.h"
#include "goom/spimpl.h"

#include <string>
#include <vector>

export module Goom.VisualFx.ImageFx;

import Goom.Utils;
import Goom.VisualFx.VisualFxBase;

export namespace GOOM::VISUAL_FX
{
class FxHelper;

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
