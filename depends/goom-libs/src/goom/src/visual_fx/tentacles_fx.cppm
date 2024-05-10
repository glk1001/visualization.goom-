module;

#include "goom/goom_config.h"

#include <cstdint>
#include <string>
#include <vector>

namespace GOOM::COLOR
{
class RandomColorMaps;
}

export module Goom.VisualFx.TentaclesFx;

import Goom.VisualFx.VisualFxBase;
import Goom.Lib.Point2d;
import Goom.Lib.SPimpl;

export namespace GOOM::VISUAL_FX
{
class FxHelper;

class TentaclesFx : public IVisualFx
{
  static constexpr auto DOMINANT_COLOR_TYPE = 0U;
  static constexpr auto NORMAL_COLOR_TYPE   = 1U;

public:
  static constexpr uint32_t NUM_TENTACLE_COLOR_TYPES = NORMAL_COLOR_TYPE + 1;

  TentaclesFx() noexcept = delete;
  explicit TentaclesFx(FxHelper& fxHelper) noexcept;

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
  class TentaclesImpl;
  spimpl::unique_impl_ptr<TentaclesImpl> m_pimpl;
};

} // namespace GOOM::VISUAL_FX
