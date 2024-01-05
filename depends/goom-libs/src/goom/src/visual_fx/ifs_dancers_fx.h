#pragma once

#include "goom/goom_types.h"
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

class IfsDancersFx : public IVisualFx
{
public:
  enum class ColorMode : UnderlyingEnumType
  {
    _NULL = -1, // NOLINT: Need special name here
    MAP_COLORS,
    MIX_COLORS,
    REVERSE_MIX_COLORS,
    MEGA_MAP_COLOR_CHANGE,
    MEGA_MIX_COLOR_CHANGE,
    SINGLE_COLORS,
    SINE_MIX_COLORS,
    SINE_MAP_COLORS,
  };

  IfsDancersFx() noexcept = delete;
  IfsDancersFx(FxHelper& fxHelper, const UTILS::GRAPHICS::SmallImageBitmaps& smallBitmaps) noexcept;

  [[nodiscard]] auto GetFxName() const noexcept -> std::string override;

  auto Start() noexcept -> void override;
  auto Finish() noexcept -> void override;

  auto Resume() noexcept -> void override;
  auto Suspend() noexcept -> void override;
  auto Refresh() noexcept -> void override;

  auto ChangePixelBlender(const PixelBlenderParams& pixelBlenderParams) noexcept -> void override;

  auto SetWeightedColorMaps(const WeightedColorMaps& weightedColorMaps) noexcept -> void override;
  [[nodiscard]] auto GetCurrentColorMapsNames() const noexcept -> std::vector<std::string> override;

  auto ApplyToImageBuffers() noexcept -> void override;

private:
  class IfsDancersFxImpl;
  spimpl::unique_impl_ptr<IfsDancersFxImpl> m_pimpl;
};

} // namespace GOOM::VISUAL_FX
