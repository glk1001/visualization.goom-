#pragma once

#include "goom/spimpl.h"
#include "goom_visual_fx.h"

#include <memory>

namespace GOOM
{

namespace COLOR
{
class RandomColorMaps;
}

namespace UTILS::GRAPHICS
{
class SmallImageBitmaps;
}

namespace VISUAL_FX
{
class FxHelper;

class FlyingStarsFx : public IVisualFx
{
public:
  enum class ColorMode
  {
    MIX_COLORS,
    REVERSE_MIX_COLORS,
    SIMILAR_LOW_COLORS,
    SINE_MIX_COLORS,
    _num // unused and must be last
  };

  FlyingStarsFx() noexcept = delete;
  FlyingStarsFx(const FxHelper& fxHelper,
                const UTILS::GRAPHICS::SmallImageBitmaps& smallBitmaps) noexcept;

  [[nodiscard]] auto GetFxName() const noexcept -> std::string override;

  auto Start() noexcept -> void override;
  auto Finish() noexcept -> void override;

  auto Resume() noexcept -> void override;
  auto Suspend() noexcept -> void override;

  auto SetWeightedColorMaps(const WeightedColorMaps& weightedColorMaps) noexcept -> void override;

  auto ApplyMultiple() noexcept -> void override;

private:
  class FlyingStarsImpl;
  spimpl::unique_impl_ptr<FlyingStarsImpl> m_pimpl;
};

} // namespace VISUAL_FX
} // namespace GOOM
