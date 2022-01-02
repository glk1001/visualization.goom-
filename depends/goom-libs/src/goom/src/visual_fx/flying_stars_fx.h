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

namespace UTILS
{
class SmallImageBitmaps;
}

namespace VISUAL_FX
{
class FxHelpers;

class FlyingStarsFx : public IVisualFx
{
public:
  enum class ColorMode
  {
    MIX_COLORS,
    REVERSE_MIX_COLORS,
    SIMILAR_LOW_COLORS,
    SINE_MIX_COLORS,
    _NUM // unused and must be last
  };

  FlyingStarsFx() noexcept = delete;
  FlyingStarsFx(const FxHelpers& fxHelpers, const UTILS::SmallImageBitmaps& smallBitmaps) noexcept;

  [[nodiscard]] auto GetFxName() const -> std::string override;

  void Start() override;

  void Resume() override;
  void Suspend() override;

  void SetWeightedColorMaps(std::shared_ptr<COLOR::RandomColorMaps> weightedMaps);
  void SetWeightedLowColorMaps(std::shared_ptr<COLOR::RandomColorMaps> weightedMaps);

  void ApplyMultiple();

  void Finish() override;

private:
  class FlyingStarsImpl;
  spimpl::unique_impl_ptr<FlyingStarsImpl> m_fxImpl;
};

} // namespace VISUAL_FX
} // namespace GOOM
