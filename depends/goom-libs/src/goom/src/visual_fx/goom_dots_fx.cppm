module;

#include <cstdint>
#include <string>
#include <vector>

export module Goom.VisualFx.GoomDotsFx;

import Goom.Color.RandomColorMaps;
import Goom.Utils.Graphics.SmallImageBitmaps;
import Goom.VisualFx.FxHelper;
import Goom.VisualFx.VisualFxBase;
import Goom.Lib.SPimpl;

export namespace GOOM::VISUAL_FX
{

class GoomDotsFx : public IVisualFx
{
public:
  GoomDotsFx() noexcept = delete;
  GoomDotsFx(FxHelper& fxHelper, const UTILS::GRAPHICS::SmallImageBitmaps& smallBitmaps) noexcept;

  [[nodiscard]] auto GetFxName() const noexcept -> std::string override;

  auto Start() noexcept -> void override;
  auto Finish() noexcept -> void override;

  auto Resume() noexcept -> void override;
  auto Suspend() noexcept -> void override;

  auto ChangePixelBlender(const PixelBlenderParams& pixelBlenderParams) noexcept -> void override;

  static constexpr uint32_t NUM_DOT_TYPES = 5;
  auto SetWeightedColorMaps(const WeightedColorMaps& weightedColorMaps) noexcept -> void override;
  [[nodiscard]] auto GetCurrentColorMapsNames() const noexcept -> std::vector<std::string> override;

  auto ApplyToImageBuffers() noexcept -> void override;

private:
  class GoomDotsFxImpl;
  spimpl::unique_impl_ptr<GoomDotsFxImpl> m_pimpl;
};

} // namespace GOOM::VISUAL_FX
